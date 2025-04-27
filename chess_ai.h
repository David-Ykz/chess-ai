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
        Move killerMoves[64][2];

        int numNegamaxSearches = 0;
        int numQuiescenceSearches = 0;
        int numPruned = 0;
        int numTranspositionTableHits = 0;
        int maxDepthSearched = 0;

        const int CHECKMATE_SCORE = 64000;
        const int MAX_NUM_EXTENSIONS = 16;
        int PIECE_VALUES[14] = {100, 300, 300, 500, 900, 0, 0, 0, -100, -300, -300, -500, -900, 0};
    public:
        ChessAI(Position& p) : position(p), transpositionTable(1048576) {}
        void printDebug() {
            cout << "Number of negamax searches: " << numNegamaxSearches << endl;
            cout << "Number of quiescence searches: " << numQuiescenceSearches << endl;
            cout << "Number of nodes pruned: " << numPruned << endl;
            cout << "Number of transposition table hits: " << numTranspositionTableHits << endl;
            cout << "Max depth searched: " << maxDepthSearched << endl;
        }
        template<Color Us>
        vector<pair<int, Move>> orderMoves(MoveList<Us>& legalMoves, int ply, bool filterCaptures) {
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
                    if (killerMoves[ply][0] == move || killerMoves[ply][1] == move) {
                        moveScore += 1000;
                    }
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
            if (ply > maxDepthSearched) {
                maxDepthSearched = ply;
            }
            if (ply > 0) {
                // TODO: implement repetition table
                // if (repetitionTable.contains(position.get_hash())) { return 0 };
            }
            alpha = max(alpha, -CHECKMATE_SCORE + ply);
            beta = min(beta, CHECKMATE_SCORE - ply);
            if (alpha >= beta) {
                return alpha;
            }

            TTEntry* entry = transpositionTable.probe(position.get_hash());
            if (entry != nullptr && entry->depth >= depth) {
                ++numTranspositionTableHits;
                int storedEval = entry->eval;
                int bound = entry->bound;
                if (bound == EXACT) {
                    if constexpr (Us == WHITE) {
                        return -storedEval;
                    } else {
                        return storedEval;
                    }
                } else if (bound == UPPER_BOUND && storedEval <= alpha) {
                    return storedEval;
                } else if (bound == LOWER_BOUND && storedEval >= beta) {
                    return storedEval;
                }
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

            Bound evaluationBound = UPPER_BOUND;
            vector<pair<int, Move>> orderedMoves = orderMoves<Us>(legalMoves, ply, false);
            Move bestMove = orderedMoves[0].second;
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
                if (extensions == 0 && depth >= 3 && i >= 3 && !move.is_capture()) {
                    eval = -negamaxSearch<~Us>(ply + 1, depth - 2, -alpha - 1, -alpha, numExtensions);
                    needsFullSearch = eval > alpha;
                }
                if (needsFullSearch) {
                    eval = -negamaxSearch<~Us>(ply + 1, depth - 1 + extensions, -beta, -alpha, numExtensions + extensions);
                }
                position.undo<Us>(move);

                if (eval >= beta) {
                    transpositionTable.store(position.get_hash(), depth, beta, LOWER_BOUND, move);
                    if (!move.is_capture() && killerMoves[ply][0] != move) {
                        killerMoves[ply][1] = killerMoves[ply][0];
                        killerMoves[ply][0] = move;
                    }
                    // TODO: killer moves and history heuristic

                    // repetitionTable.TryPop() ???
                    ++numPruned;
                    return beta;
                }
                if (eval > alpha) {
                    evaluationBound = EXACT;
                    bestMove = orderedMoves[i].second;
                    alpha = eval;
                }
            }
            if (ply > 0) {
                // repetitionTable.TryPop();
            }
            transpositionTable.store(position.get_hash(), depth, alpha, evaluationBound, bestMove);
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
            vector<pair<int, Move>> orderedMoves = orderMoves<Us>(legalMoves, -1, true);
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

        template<Color Us>
        void makeMove() {
            numNegamaxSearches = 0;
            numQuiescenceSearches = 0;
            numPruned = 0;
            numTranspositionTableHits = 0;
            maxDepthSearched = 0;
        
            Color turn = position.turn();
            int evaluation;
            chrono::steady_clock::time_point begin = chrono::steady_clock::now();
            evaluation = negamaxSearch<Us>(0, 6, -64000, 64000, 0);
            std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
            auto diff = end - begin;
            cout << "Evaluation: " << evaluation << endl;
            printDebug();
            cout << "Time taken: " << chrono::duration_cast<std::chrono::microseconds>(diff).count()/1000000.0 << " seconds" << endl;
        }
};