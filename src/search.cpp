#include "../include/search.h"

void Search::orderMoves(Movelist &moves) {
    for (int i = 0; i < moves.size(); i++) {
        Piece victim = board.at(moves[i].to());
        Piece attacker = board.at(moves[i].from());
        moves[i].setScore(mvv_lva[attacker][victim]);
    }
    sort(moves.begin(), moves.end(), [](const auto& a, const auto& b) {
        return a.score() > b.score();
    });
}

void Search::orderMoves(Movelist &moves, int ply) {
    for (int i = 0; i < moves.size(); i++) {
        Piece victim = board.at(moves[i].to());
        Piece attacker = board.at(moves[i].from());
        if (victim != Piece::NONE) {
            moves[i].setScore(mvv_lva[attacker][victim]);
        } else if (killerMoves[ply][0] == moves[i]) {
            moves[i].setScore(killerBonuses[0]);
        } else if (killerMoves[ply][1] == moves[i]) {
            moves[i].setScore(killerBonuses[1]);
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
    int eval = evaluator.evaluate(board);
    if (eval >= beta) return beta;
    if (eval > alpha) alpha = eval;

    Movelist moves;
    movegen::legalmoves<movegen::MoveGenType::CAPTURE>(moves, board);
    orderMoves(moves);
    for (int i = 0; i < moves.size(); i++) {
        Move move = moves[i];
        board.makeMove(move);
        int eval = -quiescence(-beta, -alpha);
        board.unmakeMove(move);
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
    if (ply > 0 && board.isRepetition(1)) return 0;

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
    orderMoves(moves, ply);

    for (int i = 0; i < moves.size(); i++) {
        Move move = moves[i];
        board.makeMove(move);
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

        board.unmakeMove(move);
        if (outOfTime) return 0;
        if (eval > bestEval) {
            bestEval = eval;
            if (ply == 0) rootMove = move;

            if (eval > alpha) {
                alpha = eval;
                // Fail soft
                if (alpha >= beta) {
                    if (!board.isCapture(move) && killerMoves[ply][0] != move) {
                        killerMoves[ply][1] = killerMoves[ply][0];
                        killerMoves[ply][0] = move;
                    }
                    break;
                };
            }
        }
    }

    return bestEval;
}

SearchResult Search::negamax(int depth) {
    SearchResult result;
    numNodes = 0;
    uint64_t start = tick();
    result.eval = negamax(0, depth, -INFINITY, INFINITY);
    uint64_t stop = tick();
    result.timeTakenMs = stop - start;
    result.bestMove = rootMove;
    result.numNodes = numNodes;
    result.depth = depth;
    return result;
}

SearchResult Search::search() {
    SearchResult result;
    double totalTime = 0;
    uint64_t totalNodes = 0;
    rootMove = Move();
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
            // cerr << res << endl;
        }
        if (outOfTime) break;
        result = res;
    }
    result.timeTakenMs = totalTime;
    result.numNodes = totalNodes;

    // if (debug) {
        // sendDebugInfo(result);
        // cerr << "Overall Search - " << result << endl;
    // }

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