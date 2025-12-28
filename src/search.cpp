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
    uint64_t start = tick();
    result.eval = negamax(0, depth, -INFINITY, INFINITY);
    uint64_t stop = tick();
    result.timeTaken = (stop - start) * 1.0 / 1000;
    result.bestMove = rootMove;
    result.numNodes = numNodes;
    result.depth = depth;
    return result;
}

SearchResult Search::search() {
    cout << board << endl;
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
        totalTime += res.timeTaken;
        if (debug) {
            sendDebugInfo(res);
            // cerr << res << endl;
        }
        if (outOfTime) break;
        result = res;
    }
    result.timeTaken = totalTime;
    result.numNodes = totalNodes;

    // if (debug) {
        // sendDebugInfo(result);
        // cerr << "Overall Search - " << result << endl;
    // }

    return result;
}

void Search::sendDebugInfo(SearchResult &result) {
    cout << "info depth " << result.depth;
    cout << " time " << result.timeTaken;
    cout << " nodes " << result.numNodes;
    cout << " score cp " << result.eval;
    cout << " pv " << uci::moveToUci(result.bestMove);
    cout << endl;
}