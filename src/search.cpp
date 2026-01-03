#include "../include/search.h"

bool Search::see(Move move, int threshold) {
    Square fromSquare = move.from();
    Square toSquare   = move.to();

    PieceType target = board.at<PieceType>(toSquare);

    if (move.typeOf() == Move::ENPASSANT) {
        target = PieceType::PAWN;
    }

    int value = pieceValues[target] - threshold;

    if (value < 0) return false;

    PieceType attacker = board.at<PieceType>(fromSquare);
    value -= pieceValues[attacker];

    if (value >= 0) return true;

    Bitboard occupied = board.occ();
    occupied ^= Bitboard::fromSquare(fromSquare);
    occupied |= Bitboard::fromSquare(toSquare);

    Bitboard attackers = attacks::attackers(board, Color::WHITE, toSquare) | attacks::attackers(board, Color::BLACK, toSquare);    
    attackers &= occupied;

    Bitboard queens  = board.pieces(PieceType::QUEEN);
    Bitboard bishops = board.pieces(PieceType::BISHOP) | queens;
    Bitboard rooks   = board.pieces(PieceType::ROOK)   | queens;

    Color st = ~board.at(fromSquare).color();

    while (true) {
        attackers &= occupied;

        Bitboard myAttackers = attackers & board.us(st);

        if (myAttackers.empty()) {
            break;
        }

        PieceType pt = PieceType::NONE;
        
        const PieceType types[] = {PieceType::PAWN, PieceType::KNIGHT, PieceType::BISHOP, PieceType::ROOK, PieceType::QUEEN, PieceType::KING};
        
        for (PieceType type : types) {
            if ((myAttackers & board.pieces(type)).count()) {
                pt = type;
                break;
            }
        }

        st = ~st;
        
        value = -value - 1 - pieceValues[pt];

        if (value >= 0) {
            if (pt == PieceType::KING && (attackers & board.us(st))) {
                st = ~st; 
            }
            break;
        }

        Bitboard specificAttackerBB = myAttackers & board.pieces(pt);
        Square attackerSq = Square(specificAttackerBB.lsb());
        
        occupied ^= Bitboard::fromSquare(attackerSq);

        if (pt == PieceType::PAWN || pt == PieceType::BISHOP || pt == PieceType::QUEEN)
            attackers |= attacks::bishop(toSquare, occupied) & bishops;
        
        if (pt == PieceType::ROOK || pt == PieceType::QUEEN)
            attackers |= attacks::rook(toSquare, occupied) & rooks;
    }

    return st != board.at(fromSquare).color();
}


// Order capture moves
void Search::orderMoves(Movelist &moves) {
    for (int i = 0; i < moves.size(); i++) {
        Piece victim = board.at(moves[i].to());
        Piece attacker = board.at(moves[i].from());
        moves[i].setScore(mvv_lva[attacker][victim] + GOOD_CAPTURE_BONUS * see(moves[i], -107));
    }
    sort(moves.begin(), moves.end(), [](const auto& a, const auto& b) {
        return a.score() > b.score();
    });
}

void Search::orderMoves(Movelist &moves, int ply, Move ttMove) {
    for (int i = 0; i < moves.size(); i++) {
        if (moves[i] == ttMove) {
            moves[i].setScore(TT_MOVE_BONUS);
            continue;
        }
        Piece victim = board.at(moves[i].to());
        Piece attacker = board.at(moves[i].from());
        if (victim != Piece::NONE) {
            moves[i].setScore(mvv_lva[attacker][victim] + GOOD_CAPTURE_BONUS * see(moves[i], -107));
        } else if (ss[ply].killers[0] == moves[i]) {
            moves[i].setScore(killerBonuses[0]);
        } else if (ss[ply].killers[1] == moves[i]) {
            moves[i].setScore(killerBonuses[1]);
        // } else {
        //     moves[i].setScore(history[board.sideToMove()][moves[i].from().index()][moves[i].to().index()]);
        }
    }
    sort(moves.begin(), moves.end(), [](const auto& a, const auto& b) {
        return a.score() > b.score();
    });
}

int Search::quiescence(int alpha, int beta) {
    numNodes++;
    if ((numNodes & 2047) == 0) {
        outOfTime = checkTime();
        if (outOfTime) return 0;
    }

    if (board.isRepetition()) return 0;

    int eval = evaluator.evaluate(board);
    if (eval >= beta) return beta;
    if (eval > alpha) alpha = eval;

    Movelist moves;
    movegen::legalmoves<movegen::MoveGenType::CAPTURE>(moves, board);
    orderMoves(moves);
    for (int i = 0; i < moves.size(); i++) {
        Move move = moves[i];
        board.makeMove(move, evaluator.nnue);
        int eval = -quiescence(-beta, -alpha);
        board.unmakeMove(move, evaluator.nnue);
        if (outOfTime) return 0;
        // Fail hard
        if (eval >= beta) return beta;
        if (eval > alpha) alpha = eval;
    }
    return alpha;
}

int Search::negamax(int ply, int depth, int alpha, int beta) {
    if (depth == 0) return quiescence(alpha, beta);

    numNodes++;

    if ((numNodes & 2047) == 0) {
        outOfTime = checkTime();
        if (outOfTime) return 0;
    }

    // Threefold repetition
    if (ply && board.isRepetition()) {
        return 0;
    }

    // Check transposition table
    bool ttHit = false;
    TTEntry *entry = tt.probe(board.hash(), ttHit);
    const int ttScore = ttHit ? convertTTScore(entry->score, ply) : 0;

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
    bool inCheck = board.inCheck();
    if (!inCheck && depth >= 3) {
        board.makeNullMove();
        int score = -negamax(ply + 1, depth - 3, -beta, -beta + 1);
        board.unmakeNullMove();
        if (score >= beta) return score;
    }

    Movelist moves;
    movegen::legalmoves(moves, board);

    if (moves.size() == 0) return inCheck ? -(INFINITY - ply) : 0;

    int bestEval = -INFINITY;
    int oldAlpha = alpha;
    Move bestMove = Move();
    orderMoves(moves, ply, ttHit ? entry->move : Move());
    tt.prefetch(board.hash());

    for (int i = 0; i < moves.size(); i++) {
        Move move = moves[i];
        ss[ply].currentMove = move;
        board.makeMove(move, evaluator.nnue);
        int eval;

        // Late move reduction
        bool needsFullSearch = true;
        if (i > 2 && depth > 2) {
            needsFullSearch = false;
            eval = -negamax(ply + 1, depth - 2, -alpha - 1, -alpha);
            needsFullSearch = (eval > alpha);
        }

        if (needsFullSearch) {
            eval = -negamax(ply + 1, depth - 1, -beta, -alpha);
        }

        board.unmakeMove(move, evaluator.nnue);
        if (outOfTime) return 0;
        if (eval > bestEval) {
            bestEval = eval;
            bestMove = move;
            if (ply == 0) {
                rootMove = move;
                rootEval = eval;
            }

            if (eval > alpha) {
                alpha = eval;
                // Fail soft
                if (alpha >= beta) {
                    if (!board.isCapture(move)) {
                        if (ss[ply].killers[0] != move) {
                            ss[ply].killers[1] = ss[ply].killers[0];
                            ss[ply].killers[0] = move;
                        }

                        // history[board.sideToMove()][move.from().index()][move.to().index()] += depth * depth;
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
    tt.store(board.hash(), depth, convertTTScore(bestEval, ply), flag, bestMove);

    return bestEval;
}

SearchResult Search::negamax(int depth) {
    SearchResult result;
    numNodes = 0;
    uint64_t start = tick();
    // Aspiration windows
    // bool needsFullSearch = true;
    // if (depth > 1) {
    //     for (int window = 4; window <= 1024; window <<= 4) {
    //         int alpha = result.eval - window;
    //         int beta = result.eval + window;
    //         result.eval = negamax(0, depth, alpha, beta);
    //         if (alpha <= result.eval && result.eval <= beta) {
    //             needsFullSearch = false;
    //             break;
    //         }
    //     }
    // }
    // if (needsFullSearch) {
        negamax(0, depth, -INFINITY, INFINITY);
    // }


    uint64_t stop = tick();
    result.timeTakenMs = stop - start;
    result.bestMove = rootMove;
    result.eval = rootEval;
    result.numNodes = numNodes;
    result.depth = depth;
    return result;
}

SearchResult Search::search() {
    SearchResult result;
    double totalTime = 0;
    uint64_t totalNodes = 0;
    ttHits = 0;
    rootMove = Move();
    rootEval = -INFINITY;
    outOfTime = false;
    startTime = tick();
    stopTime = startTime + thinkingTimeMs;
    // Iterative deepening
    for (int i = 1; i < 128; i++) {
        SearchResult res = negamax(i);
        totalNodes += res.numNodes;
        totalTime += res.timeTakenMs;
        if (debug) {
            sendDebugInfo(res);
        }
        result = res;
        if (outOfTime) break;
    }
    result.timeTakenMs = totalTime;
    result.numNodes = totalNodes;

    return result;
}

void Search::sendDebugInfo(SearchResult &result) {
    cout << "info depth " << result.depth;
    cout << " time " << result.timeTakenMs;
    cout << " nodes " << result.numNodes;
    cout << " score cp " << result.eval;
    cout << " pv " << uci::moveToUci(result.bestMove);
    cout << endl;
}