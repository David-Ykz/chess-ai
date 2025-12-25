using namespace chess;
using namespace std;

#include "chess-library/include/chess.hpp"
#include <vector>
#include <cstdint>

enum Bound { LOWER_BOUND, UPPER_BOUND, EXACT };

struct TTEntry {
    uint64_t zobristHash = 0;
    uint8_t depth = 0;
    int score = 0;
    bool isValid = false;
    Bound bound = EXACT;
    Move move;
};

class TranspositionTable {
public:
    vector<TTEntry> table;
    size_t tableSize;

    TranspositionTable(int exponent) {
        tableSize = 1 << exponent;
        table.resize(tableSize);
    }


    // TranspositionTable(size_t size) : tableSize(size) {
    //     table.resize(tableSize);
    // }

    inline size_t index(uint64_t zobristHash) const {
        return zobristHash & (tableSize - 1);
    }

    inline TTEntry* probe(uint64_t zobristHash) {
        TTEntry& entry = table[index(zobristHash)];
        if (entry.isValid && entry.zobristHash == zobristHash) {
            return &entry;
        }
        return nullptr;
    }

    inline void store(uint64_t zobristHash, uint8_t depth, int score, Bound bound, Move bestMove) {
        size_t idx = index(zobristHash);
        TTEntry& entry = table[idx];

        if (!entry.isValid || entry.zobristHash != zobristHash || depth >= entry.depth) {
            entry = {zobristHash, depth, score, true, bound, bestMove};
        }
    }
    
    void clear() {
        table.clear();
    }
};