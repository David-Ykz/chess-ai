using namespace chess;
using namespace std;

#include "chess.hpp"
#include <vector>
#include <cstdint>

enum Flag : uint8_t { NONE, BETA, ALPHA, EXACT };

struct alignas(16) TTEntry {
    uint64_t key;
    Move move;
    int16_t score;
    uint8_t depth;
    uint8_t flag : 2;
    uint8_t age : 6;
};

class TranspositionTable {
private:
    vector<TTEntry> table;

public:
    int currentAge;

    TranspositionTable(size_t sizeMB = 256) {
        clear();
        size_t newSize = sizeMB * 1024 * 1024 / sizeof(TTEntry);
        table.resize(newSize, TTEntry());
    }

    inline size_t index(uint64_t key) {
        // return key & (table.size() - 1);
        return key % table.size();
    }

    inline TTEntry* probe(uint64_t key, bool &ttHit) {
        TTEntry &entry = table[index(key)];
        ttHit = key == entry.key;
        return &entry;
    }

    inline void store(uint64_t key, uint8_t depth, int score, Flag flag, Move bestMove) {
        TTEntry &entry = table[index(key)];
        if (!entry.key || entry.age != currentAge || entry.depth <= depth) {
            entry.key = key;
            entry.age = currentAge;
            entry.flag = flag;
            entry.depth = depth;
            entry.move = bestMove;
            entry.score = (int16_t)score;
        }
    }
    
    inline void prefetch(uint64_t key) {
        __builtin_prefetch(&table[index(key)]);
    }

    void clear() {
        currentAge = 0;
        fill(table.begin(), table.end(), TTEntry());
    }
};