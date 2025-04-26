#pragma once
#include "eval_move.h"
#include "transposition_table.h"
#include "pst.h"
#include <limits>
#include <algorithm>
#include <chrono>

class ChessAI {
    private:
        Position& position;
        PST tables;
        TranspositionTable transpositionTable;

        int numMinimaxSearches = 0;
        int numNegamaxSearches = 0;
        int numQuiescenceSearches = 0;
        int numPruned = 0;
        int numTranspositionTableHits = 0;

        const int CHECKMATE_SCORE = 64000;
        const int MAX_NUM_EXTENSIONS = 16;
        int PIECE_VALUES[14] = {100, 300, 300, 500, 900, 0, 0, 0, -100, -300, -300, -500, -900, 0};
    public:
        ChessAI(Position& p) : position(p), transpositionTable(1000000) {}
        void printDebug() {
            cout << "Number of negamax searches: " << numNegamaxSearches << endl;
            cout << "Number of quiescence searches: " << numQuiescenceSearches << endl;
            cout << "Number of nodes pruned: " << numPruned << endl;
            cout << "Number of transposition table hits: " << numTranspositionTableHits << endl;
            transpositionTable.print();
        }
        template<Color Us>
        vector<pair<int, Move>> orderMoves(MoveList<Us>& legalMoves, bool filterCaptures) {
            vector<pair<int, Move>> orderedMoves;
            for (Move move : legalMoves) {
                int moveScore = 0;
                if (move.is_capture()) {
                    Piece from = position.at(move.from());
                    Piece to = position.at(move.to());
                    moveScore = 10 * abs(PIECE_VALUES[to]) - abs(PIECE_VALUES[from]);
                    orderedMoves.push_back({moveScore, move});
                } else if (!filterCaptures) {
                    if ((pawn_attacks<~Us>(position.bitboard_of(~Us, PAWN)) & SQUARE_BB[move.to()]) > 0) {
                        moveScore -= 100;
                    }
                    // position.play<Us>(move);
                    // if (position.in_check<~Us>()) {
                    //     moveScore += 10000;
                    // }
                    // position.undo<Us>(move);
                    orderedMoves.push_back({moveScore, move});
                }
            }
            sort(orderedMoves.begin(), orderedMoves.end(), [](const auto& a, const auto& b)
            {
                return a.first > b.first;
            });
            return orderedMoves;
   
        }

        template<Color Us>
        int negamaxSearch(int ply, int depth, int alpha, int beta, int numExtensions) {
            if (ply > 0) {
                // TODO: implement repetition table
                // if (repetitionTable.contains(position.get_hash())) { return 0 };
            }
            alpha = max(alpha, -CHECKMATE_SCORE + ply);
            beta = min(beta, CHECKMATE_SCORE - ply);
            if (alpha >= beta) {
                return alpha;
            }

            if (depth == 0) {
                return quiescenceSearch<Us>(alpha, beta);
            }

            ++numNegamaxSearches;
            MoveList<Us> legalMoves(position);
            if (legalMoves.size() == 0) {
                if (position.in_check<Us>()) { // Checkmate
                    return -(CHECKMATE_SCORE - ply);
                } else { // Stalemate
                    return 0;
                }
            }

            if (ply > 0) {
                // TODO: add repetition table logic
                // repetitionTable.store(position.get_hash(), prevWasCapture, prevWasPawnMove);
            }

            vector<pair<int, Move>> orderedMoves = orderMoves<Us>(legalMoves, false);
//            for (const auto& orderedMove : orderedMoves) {
    //                Move move = orderedMove.second;
            for (int i = 0; i < orderedMoves.size(); i++) {
                Move move = orderedMoves[i].second;
                int extensions = 0;
                position.play<Us>(move);
                // If the move is interesting, look 1 ply further
                // Note: this increases search times drastically, but should be worth it
                if (numExtensions < MAX_NUM_EXTENSIONS) {
                    if (position.in_check<~Us>()) {
                        extensions = 1;
                    }
                }
                int eval = 0;
                bool needsFullSearch = true;
                // Search moves with a low move score at a lower depth and tighter window
                if (extensions == 0 && depth >= 3 && i >= 5 && !move.is_capture()) {
                    eval = -negamaxSearch<~Us>(ply + 1, depth - 2, -alpha - 1, -alpha, numExtensions);
                    needsFullSearch = eval > alpha;
                }
                if (needsFullSearch) {
                    eval = -negamaxSearch<~Us>(ply + 1, depth - 1 + extensions, -beta, -alpha, numExtensions + extensions);
                }
                position.undo<Us>(move);

                if (eval >= beta) {
                    // TODO: add bounds for transposition table
                    // transpositionTable.store(position.get_hash(), depth, ply, beta, LOWER_BOUND, move);
                    
                    // TODO: killer moves and history heuristic

                    // repetitionTable.TryPop() ???
                    ++numPruned;
                    return beta;
                }
                if (eval > alpha) {
                    alpha = eval;

                }
            }
            if (ply > 0) {
                // repetitionTable.TryPop();
            }
            // TODO: transposition table store logic

            return alpha;
            
        }

        template<Color Us>
        int quiescenceSearch(int alpha, int beta) {
            ++numQuiescenceSearches;
            int eval = evaluate();
            if (eval >= beta) {
                ++numPruned;
                return beta;
            }
            if (eval > alpha) {
                alpha = eval;
            }
            MoveList<Us> legalMoves(position);
            vector<pair<int, Move>> orderedMoves = orderMoves<Us>(legalMoves, true);
            for (int i = 0; i < orderedMoves.size(); i++) {
                Move move = orderedMoves[i].second;
                position.play<Us>(move);
                eval = -quiescenceSearch<~Us>(-beta, -alpha);
                position.undo<Us>(move);

                if (eval >= beta) {
                    ++numPruned;
                    return beta;
                }
                if (eval > alpha) {
                    alpha = eval;
                }
            }
            return alpha;
        }

        template <Color Us>
        int quiescent(int alpha, int beta) {
            int standPat = evaluate();
            int bestValue = standPat;
            if (standPat >= beta) {
                ++numPruned;
                return standPat;
            }
            if (standPat + 100 < alpha) {
                return standPat;
            }

            if (alpha < standPat) {
                alpha = standPat;
            }

            MoveList<Us> legalMoves(position);
            vector<pair<int, Move>> orderedMoves = orderMoves(legalMoves, true);
            for (const auto& orderedMove : orderedMoves) {
                const Move move = orderedMove.second;
                position.play<Us>(move);
                int score = -quiescent<~Us>(-beta, -alpha);
                position.undo<Us>(move);
                if (score >= beta) {
                    ++numPruned;
                    return bestValue;
                }
                if (score > bestValue) {
                    bestValue = score;
                }
                if (score > alpha) {
                    alpha = score;
                }
            }
            return bestValue;        
        }

        template <Color Us>
        int search(int depth, int alpha, int beta) {
            MoveList<Us> legalMoves(position);
            if (legalMoves.size() == 0) {
                if (position.in_check<Us>()) {
                    return -CHECKMATE_SCORE - depth;
                }
                return 0;
            }
            
            const uint64_t hash = position.get_hash();
            if (transpositionTable.contains(hash)) {
                TTEntry entry = transpositionTable.get(hash);
                if (entry.depth >= depth) {
                    ++numTranspositionTableHits;
                    if constexpr (Us == WHITE) {
                        return -entry.eval;
                    } else {
                        return entry.eval;
                    }
                }
            }

            
            if (depth == 0) {
                ++numQuiescenceSearches;
                return quiescent<Us>(alpha, beta);
            }
            int bestValue = -64000;
            ++numMinimaxSearches;
            vector<pair<int, Move>> orderedMoves = orderMoves(legalMoves, false);
            for (const auto& orderedMove : orderedMoves) {
                const Move move = orderedMove.second;
                position.play<Us>(move);
                int score = -search<~Us>(depth - 1, -beta, -alpha);
                position.undo<Us>(move);
                if (score >= beta) {
                    ++numPruned;
                    return beta;
                }

                if (score > bestValue) {
                    bestValue = score;
                    // if (score >= beta) {
                    //     ++numPruned;
                    //     return bestValue;
                    // }
                        if (score > alpha) {
                        alpha = score;
                    }
                }
            }
            transpositionTable.store(hash, depth, bestValue, -1); // We don't store bestMove here since it's not needed for exact evaluation
            return bestValue;
        }
        
        int evaluate() {
            int evaluation = 0;

            for (int i = WHITE_PAWN; i < NO_PIECE; ++i) {
                Piece piece = static_cast<Piece>(i);
                Bitboard bitboard = position.bitboard_of(piece);
                while (bitboard) {
                    int square = __builtin_ctzll(bitboard);
                    bitboard &= bitboard - 1;
                    evaluation += PIECE_VALUES[i] + tables.pst[i][square];
                }

            }
            if (position.turn() == BLACK) {
                return -evaluation;
            }

            return evaluation;
        }

        void makeMove() {
            numMinimaxSearches = 0;
            numQuiescenceSearches = 0;
            numPruned = 0;
        
            Color turn = position.turn();
            int evaluation;
            chrono::steady_clock::time_point begin = chrono::steady_clock::now();
            if (turn == WHITE) {
                evaluation = negamaxSearch<WHITE>(0, 7, -64000, 64000, 0);

            } else {
                cout << "start" << endl;
                evaluation = search<BLACK>(3, -64000, 64000);
                cout << "end" << endl;
            }
            std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
            auto diff = end - begin;
            cout << "Evaluation: " << evaluation << endl;
            printDebug();
            cout << "Time taken: " << chrono::duration_cast<std::chrono::microseconds>(diff).count()/1000000.0 << " seconds" << endl;
        }
};