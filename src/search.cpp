#include "../include/search.h"

/* Returns whether a capture is optimal or not based off static exchange evaluation */
bool see(Board &board, Move move, int threshold) {
    Square fromSquare = move.from();
    Square toSquare   = move.to();
    PieceType target = board.at<PieceType>(toSquare);

    // Check if a capture is clearly good or bad
    if (move.typeOf() == Move::ENPASSANT) target = PieceType::PAWN;
    
    int value = pieceValues[target] - threshold;
    if (value < 0) return false;

    PieceType attacker = board.at<PieceType>(fromSquare);
    value -= pieceValues[attacker];
    if (value >= 0) return true;

    // Get bitboards for pieces
    Bitboard occupied = board.occ();
    occupied ^= Bitboard::fromSquare(fromSquare);
    occupied |= Bitboard::fromSquare(toSquare);
    Bitboard attackers = attacks::attackers(board, Color::WHITE, toSquare) | attacks::attackers(board, Color::BLACK, toSquare);    
    attackers &= occupied;
    Bitboard queens = board.pieces(PieceType::QUEEN);
    Bitboard bishops = board.pieces(PieceType::BISHOP) | queens;
    Bitboard rooks = board.pieces(PieceType::ROOK)   | queens;

    Color sideToMove = ~board.at(fromSquare).color();

    // SEE loop
    while (true) {
        // Find attackers
        attackers &= occupied;
        Bitboard myAttackers = attackers & board.us(sideToMove);
        if (myAttackers.empty()) break;

        // Find the cheapest attackers
        PieceType pt = PieceType::NONE;        
        for (PieceType type : types) {
            if ((myAttackers & board.pieces(type)).count()) {
                pt = type;
                break;
            }
        }

        // Compute material change
        sideToMove = ~sideToMove;
        value = -value - 1 - pieceValues[pt];
        if (value >= 0) {
            // Prevent illegal king captures
            if (pt == PieceType::KING && (attackers & board.us(sideToMove))) {
                sideToMove = ~sideToMove; 
            }
            break;
        }

        // Look for newly uncovered attackers
        Bitboard specificAttackerBB = myAttackers & board.pieces(pt);
        Square attackerSq = Square(specificAttackerBB.lsb());
        occupied ^= Bitboard::fromSquare(attackerSq);
        if (pt == PieceType::PAWN || pt == PieceType::BISHOP || pt == PieceType::QUEEN)
            attackers |= attacks::bishop(toSquare, occupied) & bishops;
        if (pt == PieceType::ROOK || pt == PieceType::QUEEN)
            attackers |= attacks::rook(toSquare, occupied) & rooks;
    }
    return sideToMove != board.at(fromSquare).color();
}

/* Sorts the move list to search strong moves first */
void orderMoves(SearchThread &st, Movelist &moves, int ply, Move ttMove) {
    /* Orders moves in the following hierarchy:
        1. moves from transposition table
        2. good captures (based off SEE)
        3. counter moves
        4. killer moves
        5. history moves
        6. bad captures (ordered with MVV-LVA)
        7. quiet moves
    */
    for (int i = 0; i < moves.size(); i++) {
        if (moves[i] == ttMove) {
            moves[i].setScore(TT_MOVE_BONUS);
            continue;
        }
        Piece victim = st.board.at(moves[i].to());
        Piece attacker = st.board.at(moves[i].from());
        if (victim != Piece::NONE) { // Captures
            moves[i].setScore(mvv_lva[attacker][victim] + GOOD_CAPTURE_BONUS * see(st.board, moves[i], -107));
        } else if (st.stack[ply].killers[0] == moves[i]) { // Killer move 1
            moves[i].setScore(KILLER_BONUS[0]);
        } else if (st.stack[ply].killers[1] == moves[i]) { // Killer move 2
            moves[i].setScore(KILLER_BONUS[1]);
        } else if (ply && st.counters[st.board.sideToMove()][st.stack[ply - 1].currentMove.from().index()][st.stack[ply - 1].currentMove.to().index()] == moves[i]) { // Counter move
            moves[i].setScore(COUNTER_BONUS);
        } else { // Quiet move
            moves[i].setScore(st.history[st.board.sideToMove()][moves[i].from().index()][moves[i].to().index()]);
        }
    }
    sort(moves.begin(), moves.end(), [](const auto& a, const auto& b) {
        return a.score() > b.score();
    });
}

/* Order capture moves, only used for quiescence search */
void orderMoves(SearchThread &st, Movelist &moves) {
    for (int i = 0; i < moves.size(); i++) {
        Piece victim = st.board.at(moves[i].to());
        Piece attacker = st.board.at(moves[i].from());
        moves[i].setScore(mvv_lva[attacker][victim] + GOOD_CAPTURE_BONUS * see(st.board, moves[i], -107));
    }
    sort(moves.begin(), moves.end(), [](const auto& a, const auto& b) {
        return a.score() > b.score();
    });
}

/* Search capture moves to avoid horizon effect */
int quiescence(SearchThread &st, int alpha, int beta) {
    st.search.numNodes++;
    
    // Check if out of time every 2048 nodes searched
    if ((st.search.numNodes & 2047) == 0) {
        if (checkTime(st.search.stopTime)) {
            st.search.outOfTime = true;
        }
        if (st.search.outOfTime) return 0;
    }

    if (st.board.isRepetition()) return 0;

    // Stand pat (eval if we do nothing)
    int eval = st.nnue.Evaluate(st.board.sideToMove());
    if (eval >= beta) return beta;
    if (eval > alpha) alpha = eval;

    Movelist moves;
    movegen::legalmoves<movegen::MoveGenType::CAPTURE>(moves, st.board);
    orderMoves(st, moves);
    for (int i = 0; i < moves.size(); i++) {
        Move move = moves[i];
        // Prune based off SEE
        if (move.score() < GOOD_CAPTURE_BONUS && i > 0) continue;
        
        st.board.makeMove(move, st.nnue);
        int eval = -quiescence(st, -beta, -alpha);
        st.board.unmakeMove(move, st.nnue);

        if (st.search.outOfTime) return 0;
        if (eval >= beta) return beta;
        if (eval > alpha) alpha = eval;
    }
    return alpha;
}

/* Main search function */
int negamax(SearchThread &st, int ply, int depth, int alpha, int beta) {
    if (depth == 0) return quiescence(st, alpha, beta);

    st.search.numNodes++;

    if ((st.search.numNodes & 2047) == 0) {
        if (checkTime(st.search.stopTime)) {
            st.search.outOfTime = true;
        }
        if (st.search.outOfTime) return 0;
    }

    // Threefold repetition
    if (ply && st.board.isRepetition(1)) return 0;

    // Check transposition table for this position
    bool ttHit = false;
    TTEntry *entry = st.tt.probe(st.board.hash(), ttHit);
    const int ttScore = ttHit ? convertTTScore(entry->score, ply) : 0;
    
    // If conditions are suitable, we can return the stored result instead of recomputing it
    if (ttHit && ply && entry->depth >= depth && ttScore != INFINITY) {
        if (entry->flag == EXACT) {
            return ttScore;
        } else if (entry->flag == BETA && ttScore >= beta) {
            return beta;
        } else if (entry->flag == ALPHA && ttScore <= alpha) {
            return alpha;
        }
    }

    // Null move pruning
    bool inCheck = st.board.inCheck();
    if (!inCheck && depth >= 3) {
        st.board.makeNullMove();
        int score = -negamax(st, ply + 1, depth/2 - 1, -beta, -beta + 1);
        st.board.unmakeNullMove();
        if (score >= beta) return score;
    }

    Movelist moves;
    movegen::legalmoves(moves, st.board);
    if (moves.size() == 0) return inCheck ? -(INFINITY - ply) : 0;

    int staticEval = st.nnue.Evaluate(st.board.sideToMove());

    // Reverse futility pruning
    int margin = 160 * depth;
    bool isPVNode = (beta - alpha) > 1;
    if (!isPVNode && !inCheck && !ttHit && staticEval >= beta + margin) return staticEval;

    // Futility pruning
    if (depth <= 2 && !inCheck && staticEval + 2 * margin < alpha) {
        return quiescence(st, alpha, beta);
    }

    int bestEval = -INFINITY;
    int oldAlpha = alpha;
    Move bestMove = Move();
    orderMoves(st, moves, ply, ttHit ? entry->move : Move());
    st.tt.prefetch(st.board.hash());

    for (int i = 0; i < moves.size(); i++) {
        Move move = moves[i];
        st.stack[ply].currentMove = move;
        st.board.makeMove(move, st.nnue);
        int eval;

        // Late move reduction
        bool needsFullSearch = true;
        if (i > 2 && depth > 2) {
            needsFullSearch = false;
            eval = -negamax(st, ply + 1, depth - reductions[min(depth, 32) - 1][min(i, 32) - 1], -alpha - 1, -alpha);
            needsFullSearch = (eval > alpha);
        }
        if (needsFullSearch) {
            eval = -negamax(st, ply + 1, depth - 1, -beta, -alpha);
        }

        st.board.unmakeMove(move, st.nnue);
        if (st.search.outOfTime) return 0;

        if (eval > bestEval) {
            bestEval = eval;
            bestMove = move;
            // Assign root move and eval
            if (ply == 0) {
                st.rootMove = move;
                st.rootEval = eval;
            }

            if (eval > alpha) {
                alpha = eval;
                if (alpha >= beta) {
                    // Update history heuristics if the move was quiet
                    if (!st.board.isCapture(move)) {
                        if (st.stack[ply].killers[0] != move) {
                            st.stack[ply].killers[1] = st.stack[ply].killers[0];
                            st.stack[ply].killers[0] = move;
                        }
                        if (ply) {
                            Move prevMove = st.stack[ply - 1].currentMove;
                            if (prevMove != Move()) {
                                st.counters[~st.board.sideToMove()][prevMove.from().index()][prevMove.to().index()] = move;
                            }
                        }

                        st.history[st.board.sideToMove()][move.from().index()][move.to().index()] = min(MAX_HISTORY_BONUS, st.history[st.board.sideToMove()][move.from().index()][move.to().index()] + depth * depth);
                    }
                    break;
                };
            }
        }
    }

    // Store position into transposition table
    Flag flag = ALPHA;
    if (bestEval >= beta) {
        flag = BETA;
    } else if (alpha != oldAlpha) {
        flag = EXACT;
    }
    st.tt.store(st.board.hash(), depth, convertTTScore(bestEval, ply), flag, bestMove);

    return bestEval;
}

/* Iterative deepening, also thread entry point */
void iterativeDeepening(SearchThread &st) {
    int prevEval = 0;
    for (int depth = st.threadID + 1; depth < MAX_PLY; depth++) {
        uint64_t startTime = tick();
        bool needsFullSearch = true;
        int delta = 16;

        if (depth > 3) {
            int alpha = max(-INFINITY, prevEval - delta);
            int beta = min(INFINITY, prevEval + delta);
            
            // Aspiration windows
            while (true) {
                int eval = negamax(st, 0, depth, alpha, beta);
                if (st.search.outOfTime) break;

                if (eval <= alpha) {
                    beta = (alpha + beta) / 2;
                    alpha = max(-INFINITY, prevEval - delta);
                } else if (eval >= beta) {
                    alpha = (alpha + beta) / 2;
                    beta = min(INFINITY, prevEval + delta);
                } else {
                    prevEval = eval;
                    break;
                }
                delta <<= 2;
            }
        } else {
            prevEval = negamax(st, 0, depth, -INFINITY, INFINITY);
        }

        if (!st.threadID) {
            uint64_t stopTime = tick();
            printInfo(depth, stopTime - startTime, st.rootEval, st.rootMove);
        }
        if (st.search.outOfTime) break;
    }
}

/* Chess engine endpoint */
SearchResult search(string fen, uint64_t allottedTime, TranspositionTable &tt) {
    SearchStatus search;
    search.startTime = tick();
    search.thinkingTimeMs = allottedTime - TIME_MARGIN;
    search.stopTime = search.startTime + search.thinkingTimeMs;
    
    cout << allottedTime << endl;
    cout << search.thinkingTimeMs << endl;
    cout << search.startTime << endl;
    cout << search.stopTime << endl;

    vector<thread> threads;
    vector<unique_ptr<SearchThread>> searchThreads;

    for (int i = 0; i < NUM_THREADS; i++) {
        unique_ptr<SearchThread> searchThread = make_unique<SearchThread>(fen, tt, search, i);
        searchThreads.push_back(move(searchThread));
        if (i) threads.emplace_back(thread(iterativeDeepening, ref(*searchThreads[i])));
    }

    iterativeDeepening(*searchThreads[0]);

    for (int i = 1; i < NUM_THREADS; i++) {
        threads[i - 1].join();
    }

    search.result = {searchThreads[0]->rootMove, searchThreads[0]->rootEval};
    return search.result;
}


void printInfo(int depth, uint64_t time, int eval, Move move) {
    cout << "info depth " << depth;
    cout << " time " << time;
    // cout << " nodes " << result.st.search.numNodes;
    cout << " score cp " << eval;
    cout << " pv " << uci::moveToUci(move);
    cout << endl;
}