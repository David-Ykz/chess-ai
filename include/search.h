#include "searchthread.h"
#include <vector>
#include <chrono>
#include <iostream>
#include <thread>


inline uint64_t tick() noexcept {
    return chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();
}
inline bool checkTime(uint64_t stopTime) {
    return tick() > stopTime;
}
inline int convertTTScore(int score, int ply) {
    if (score >= INFINITY - MAX_PLY) {
        return score - ply;
    } else if (score <= -INFINITY + MAX_PLY) {
        return score + ply;
    }
    return score;
}

bool see(Board &board, Move move, int threshold);
void orderMoves(SearchThread &st, Movelist& moves, int ply, Move move);
void orderMoves(SearchThread &st, Movelist& moves);
int quiescence(SearchThread &st, int alpha, int beta);
int negamax(SearchThread &st, int ply, int depth, int alpha, int beta);
void iterativeDeepening(SearchThread &st);
SearchResult search(string fen, uint64_t allottedTime, TranspositionTable &tt);
void printInfo(int depth, uint64_t time, int eval, Move move);