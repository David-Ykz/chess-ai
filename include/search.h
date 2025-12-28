#include "eval.h"
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
    double      timeTaken;
};

inline ostream& operator<<(ostream& os, const SearchResult& res) {
    os << "Best Move: " << res.bestMove.from() << res.bestMove.to(); 
    os << " | Eval: " << res.eval;
    os << " | Depth: " << res.depth;
    os << " | Nodes: " << res.numNodes;
    os << " | Time Taken: " << res.timeTaken;

    return os;
}


class Search {
private:
    Board board;
    Eval evaluator; 
    uint64_t numNodes;
    Move rootMove;
    const int INFINITY = 99000;
    uint64_t startTime, stopTime, thinkingTimeMs;
    const bool debug = true;
    bool outOfTime = false;

    inline uint64_t tick() noexcept {
        return chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();
    }
    inline bool checkTime() {
        return tick() > stopTime;
    }


public:
    Search(uint64_t allotedTime) {
        board = Board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        thinkingTimeMs = allotedTime;
    }
    Search(Board &externalBoard, uint64_t allotedTime) {
        board = externalBoard;
        thinkingTimeMs = allotedTime;
    }

    void orderMoves(Movelist& moves);
    int quiescence(int alpha, int beta);
    int negamax(int ply, int depth, int alpha, int beta);
    SearchResult negamax(int depth);
    SearchResult search();
    void sendDebugInfo(SearchResult &result);

};