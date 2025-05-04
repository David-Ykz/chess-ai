#include "search.h"

template <Color Us>
int Search::negamax(int ply, int depth) {
    ++nodesSearched;
    if (depth == 0) {
        return evaluator.evaluate<Us>(position);
        //        return Evaluation::evaluate<Us>(position);
    }
    int max = -64000;
    MoveList<Us> legalMoves(position);
    if (legalMoves.size() == 0) {
        if (position.in_check<Us>()) { // Checkmate
            return -(CHECKMATE_SCORE - ply);
        } else { // Stalemate
            return 0;
        }
    }
    for (Move move : legalMoves) {
        position.play<Us>(move);
        int score = -negamax<~Us>(ply + 1, depth - 1);
        position.undo<Us>(move);
        if (score > max) {
            max = score;
            if (ply == 0) {
                bestMove = move;
            }
        }
    }
    return max;
}

template <Color Us>
SearchResult Search::search() {
    nodesSearched = 0;
    int depth = 5;
    auto begin = chrono::steady_clock::now();
    int eval = negamax<Us>(0, depth);
    auto end = chrono::steady_clock::now();
    int duration = chrono::duration_cast<chrono::microseconds>(end - begin).count();
    return SearchResult(bestMove, eval, depth, nodesSearched, duration);
}

template SearchResult Search::search<WHITE>();
template SearchResult Search::search<BLACK>();