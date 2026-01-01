using namespace chess;
using namespace std;

#include "chess.hpp"
#include <vector>
#include <cstdint>

// enum Bound { LOWER_BOUND, UPPER_BOUND, EXACT };

enum Flag : uint8_t { NONE, BETA, ALPHA, EXACT };

/*
struct alignas(16) TTEntry {
    uint32_t key;     // Upper 32 bits of Zobrist hash
    Move move;        // 32-bit move
    int16_t score;    // 16-bit score
    int16_t eval;     // 16-bit static evaluation
    uint8_t depth;    // 8-bit depth
    uint8_t type;     // 8-bit: [2 bits Bound] [6 bits Age/Generation]
};

use static cast for int16?
fix indexing for 32 bit key (can also pass in uint32_t instead of uint64_t) and make it faster?
*/


struct alignas(16) TTEntry {
    uint64_t key;
    Move move;
    int16_t score;
    // int16_t eval;
    uint8_t depth;
    uint8_t flag : 2;
    uint8_t age : 6;
};

class TranspositionTable {
private:
    vector<TTEntry> table;

public:
    int currentAge;
    uint64_t numCollisions, numHits;

    TranspositionTable(size_t sizeMB = 256) {
        clear();
        size_t newSize = sizeMB * 1024 * 1024 / sizeof(TTEntry);
        table.resize(newSize, TTEntry());
        // cout << table.size() << " " << sizeof(TTEntry) << endl;
    }

    inline size_t index(uint64_t key) {
        // since TTEntry is 16 bytes and as long as we pick a TT size that is a power of 2, we can avoid using modulo
        // return key & (table.size() - 1);
        return key % table.size();
    }

    inline TTEntry* probe(uint64_t key, bool &ttHit) {
        TTEntry &entry = table[index(key)];
        ttHit = key == entry.key;
        // ttHit = (uint32_t)key == entry.key;
        numHits += ttHit;
        return &entry;
    }

    inline void store(uint64_t key, uint8_t depth, int score, int eval, Flag flag, Move bestMove) {
        TTEntry &entry = table[index(key)];
        if (!entry.key || entry.age != currentAge || entry.depth <= depth) {
            entry.key = key;
            // entry.key = (uint32_t)key;
            entry.age = currentAge;
            entry.flag = flag;
            entry.depth = depth;
            entry.move = bestMove;
            entry.score = (int16_t)score;
            // entry.eval = (int16_t)eval;
            numCollisions++;
        }
    }
    
    inline void prefetch(uint64_t key) {
    #if defined(__INTEL_COMPILER) || defined(_MSC_VER)
        _mm_prefetch((char *)addr, _MM_HINT_T0);
    #else
        __builtin_prefetch(&table[index(key)]);
    #endif
    }

    void clear() {
        currentAge = 0;
        fill(table.begin(), table.end(), TTEntry());
        // table.clear();
        numCollisions = 0;
    }
};