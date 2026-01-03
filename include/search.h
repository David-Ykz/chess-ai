#include "eval.h"
#include "transposition_table.h"
#include <vector>
#include <chrono>
#include <iostream>
#include <iomanip>
using namespace std;

struct SearchResult {
    Move        bestMove;
    int         eval;
    uint64_t    numNodes;
    int         depth;
    uint64_t    timeTakenMs;
};

inline ostream& operator<<(ostream& os, const SearchResult& res) {
    os << "Best Move: " << res.bestMove.from() << res.bestMove.to(); 
    os << " | Eval: " << res.eval;
    os << " | Depth: " << res.depth;
    os << " | Nodes: " << res.numNodes;
    os << " | Time Taken: " << res.timeTakenMs;

    return os;
}

struct SearchStack {
    Move currentMove = Move();
    Move killers[2] = {Move(), Move()};
};



class Search {
private:
    Board board;
    Eval evaluator;
    TranspositionTable &tt;
    Move rootMove;
    int rootEval;

    const int killerBonuses[2] = {8000, 4000};

    SearchStack ss[128];
    int16_t history[2][64][64];

    const int INFINITY = 32000;
    const int MAX_PLY = 128;
    const uint64_t TIME_MARGIN = 50;
    const string NNUE_NAME = "nets/nn-c288c895ea92.nnue";
    const int TT_MOVE_BONUS = 16000;
    const int GOOD_CAPTURE_BONUS = 10000;
    const int MAX_HISTORY_BONUS = 3000;

    uint64_t numNodes, ttHits, startTime, stopTime, thinkingTimeMs;
    const bool debug = true;
    bool outOfTime = false;

    inline uint64_t tick() noexcept {
        return chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();
    }
    inline bool checkTime() {
        return tick() > stopTime;
    }
    inline int log2(uint32_t x) {
        return 31 - __builtin_clz(x);
    }
    inline int convertTTScore(int score, int ply) {
        if (score >= INFINITY - MAX_PLY) {
            return score - ply;
        } else if (score <= -INFINITY + MAX_PLY) {
            return score + ply;
        }
        return score;
    }
    
    void init() {
        NNUE::Init(NNUE_NAME);
        memset(history, 0, sizeof(int16_t) * 2 * 64 * 64);
    }




public:
    Search(string_view fen, uint64_t allottedTime, TranspositionTable &tt) : tt(tt) {
        init();
        board = Board(evaluator.nnue, fen);
        thinkingTimeMs = allottedTime - TIME_MARGIN;
    }

    bool see(Move move, int threshold);
    void orderMoves(Movelist& moves);
    void orderMoves(Movelist& moves, int ply, Move move);
    int quiescence(int alpha, int beta);
    int negamax(int ply, int depth, int alpha, int beta);
    SearchResult negamax(int depth);
    SearchResult search();
    void sendDebugInfo(SearchResult &result);
};