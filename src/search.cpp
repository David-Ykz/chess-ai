#include "../include/search.h"

void Search::orderMoves(Movelist &moves) {
    for (int i = 0; i < moves.size(); i++) {
        Piece victim = board.at(moves[i].to());
        Piece attacker = board.at(moves[i].from());
        if (victim != Piece::NONE) {
            moves[i].setScore(mvv_lva[attacker][victim]);
        }
    }
    sort(moves.begin(), moves.end(), [](const auto& a, const auto& b) {
        return a.score() > b.score();
    });
}

int Search::quiescence(int alpha, int beta) {
    numNodes++;
    if ((numNodes & 2047) == 0) outOfTime = checkTime();
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

    if ((numNodes & 2047) == 0) outOfTime = checkTime();

    // Threefold repetition
    if (board.isRepetition(1)) return 0;

    Movelist moves;
    movegen::legalmoves(moves, board);

    if (moves.size() == 0) return board.inCheck() ? -(INFINITY - ply) : 0;

    int bestEval = -INFINITY;
    orderMoves(moves);

    for (int i = 0; i < moves.size(); i++) {
        Move move = moves[i];
        board.makeMove(move);
        int eval = -negamax(ply + 1, depth - 1, -beta, -alpha);
        board.unmakeMove(move);
        if (outOfTime) return 0;
        if (eval > bestEval) {
            bestEval = eval;
            if (ply == 0) rootMove = move;

            if (eval > alpha) {
                alpha = eval;
                // Fail soft
                if (alpha >= beta) break;
            }
        }
    }

    return bestEval;
}

SearchResult Search::negamax(int depth) {
    SearchResult result;
    numNodes = 0;
    outOfTime = false;
    startTime = tick();
    stopTime = startTime + thinkingTimeMs;
    result.eval = negamax(0, depth, -INFINITY, INFINITY);
    uint64_t actualStopTime = tick();
    result.timeTaken = (actualStopTime - startTime) * 1.0 / 1000;
    result.bestMove = rootMove;
    result.numNodes = numNodes;
    result.depth = depth;
    return result;
}

SearchResult Search::search() {
    SearchResult result;
    double totalTime = 0;
    uint64_t totalNodes = 0;
    // Iterative deepening
    for (int i = 0; i < 128; i++) {
        SearchResult res = negamax(i);
        totalNodes += res.numNodes;
        totalTime += res.timeTaken;
        if (debug) {
            cout << res << endl;
        }
        if (outOfTime) break;
        result = res;
    }
    result.timeTaken = totalTime;
    result.numNodes = totalNodes;
    return result;
}