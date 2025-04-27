using namespace std;

#pragma once

#include <vector>
#include <cstdint>

enum Bound { LOWER_BOUND, UPPER_BOUND, EXACT };

struct TTEntry {
    uint64_t zobristHash = 0;
    int depth = -1;
    int eval = 0;
    Bound bound = EXACT;
    Move bestMove;

    bool isValid(uint64_t hash) const {
        return zobristHash == hash;
    }
};

class TranspositionTable {
public:
    vector<TTEntry> table;
    size_t tableSize;

    TranspositionTable(size_t size) : tableSize(size) {
        table.resize(tableSize);
    }

    size_t index(uint64_t zobristHash) const {
        return zobristHash % tableSize;
    }

    TTEntry* probe(uint64_t zobristHash) {
        TTEntry& entry = table[index(zobristHash)];
        if (entry.isValid(zobristHash)) {
            return &entry;
        }
        return nullptr;
    }

    void store(uint64_t zobristHash, int depth, int eval, Bound bound, Move bestMove) {
        size_t idx = index(zobristHash);
        TTEntry& entry = table[idx];

        if (entry.zobristHash != zobristHash || depth >= entry.depth) {
            entry = {zobristHash, depth, eval, bound, bestMove};
        }
    }
};








// using namespace std;

// #pragma once

// #include <vector>
// #include <iostream>
// #include <cstdint>


// enum Bound { LOWER_BOUND, UPPER_BOUND, EXACT };

// struct TTEntry {
//     uint64_t zobristHash;
//     int depth;
//     int eval;
//     Bound bound;
//     Move bestMove;
// };

// // Transposition Table class
// class TranspositionTable {
// public:
//     vector<TTEntry> table;
//     size_t tableSize;

//     // Constructor to initialize the table size
//     TranspositionTable(size_t size) : tableSize(size) {
//         table.resize(tableSize);
//         for (int i = 0; i < tableSize; i++) {
//             table[i] = {0, -1, 0, EXACT, Move()};
//         }
//     }

//     size_t hash(uint64_t zobristHash) {
//         return zobristHash % tableSize;
//     }

//     bool contains(uint64_t zobristHash) {
//         size_t key = hash(zobristHash);
//         return table[key].depth > 0 && (table[key].zobristHash == zobristHash);
//     }

//     TTEntry get(uint64_t zobristHash) {
//         size_t key = hash(zobristHash);
//         if (table[key].zobristHash == zobristHash) {
//             return table[key];
//         }
//         return table[hash(zobristHash)];
//     }

//     // Function to store a position in the table
//     void store(uint64_t zobristHash, int depth, int eval, Bound bound, Move bestMove) {
//         TTEntry entry{zobristHash, depth, eval, bound, bestMove};
//         size_t key = hash(zobristHash);
//         TTEntry existingEntry = table[key];
//         if (zobristHash != existingEntry.zobristHash || depth > existingEntry.depth) {
//             table[key] = entry;
//         }
//     }

//     int getNumEntries() {
//         int numEntries = 0;
//         for (int i = 0; i < tableSize; i++) {
//             if (table[i].depth > 0) {
//                 ++numEntries;
//             }
//         }
//         return numEntries;
//     }
// };