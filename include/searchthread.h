#include "chess.hpp"
#include "nnue.h"
#include "transposition_table.h"
#include "constants.h"
#include <atomic>

struct SearchResult {
    Move move = Move();
    int eval = 0;
};

struct SearchStatus {
    uint64_t startTime, stopTime, thinkingTimeMs;
    atomic_uint64_t numNodes = 0;
    atomic_bool outOfTime = false;

    SearchResult result;
};

struct SearchStack {
    Move currentMove = Move();
    Move killers[2] = {Move(), Move()};
};

class SearchThread {
public:
    Board board;
    NNUE::Net nnue;
    TranspositionTable &tt;
    SearchStack stack[MAX_PLY];
    int16_t history[2][64][64];
    Move counters[2][64][64];

    SearchStatus &search;
    Move rootMove;
    int rootEval;
    int threadID;

    SearchThread(string fen, TranspositionTable &tt, SearchStatus &search, int threadID) : tt(tt), search(search), threadID(threadID) {
        NNUE::Init(NNUE_NAME);
        board = Board(nnue, fen);

        memset(history, 0, sizeof(int16_t) * 2 * 64 * 64);
        memset(counters, 0, sizeof(Move) * 2 * 64 * 64);
        memset(stack, 0, sizeof(SearchStack) * MAX_PLY);
    }
};