#pragma once
#include "eval_move.h"
#include "pst.h"
#include <limits>
#include <algorithm>
#include <chrono>

class ChessAI {
    private:
        Position& position;
        PST tables;

        int numMinimaxSearches = 0;
        int numQuiescenceSearches = 0;
        int numPruned = 0;

        const int CHECKMATE_SCORE = 64000;
        int PIECE_VALUES[14] = {100, 300, 300, 500, 900, 0, 0, 0, -100, -300, -300, -500, -900, 0};
    public:
        ChessAI(Position& p) : position(p) {}
        void printDebug() {
            cout << "Number of minimax searches: " << numMinimaxSearches << endl;
            cout << "Number of quiescence searches: " << numQuiescenceSearches << endl;
            cout << "Number of nodes pruned: " << numPruned << endl;
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
                    position.play<Us>(move);
                    if (position.in_check<~Us>()) {
                        moveScore += 10000;
                    }
                    position.undo<Us>(move);
                    orderedMoves.push_back({moveScore, move});
                }
            }
            sort(orderedMoves.begin(), orderedMoves.end(), [](const auto& a, const auto& b)
            {
                return a.first > b.first;
            });
            return orderedMoves;
   
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
                if (score > bestValue) {
                    bestValue = score;
                    if (score > alpha) {
                        alpha = score;
                    }
                    if (score >= beta) {
                        ++numPruned;
                        return bestValue;
                    }
                }
            }
            return bestValue;
        }

        template <Color Us>
        bool isDraw(MoveList<Us>& legalMoves) {
            return legalMoves.size() == 0;
        }
        
        template <Color Us>
        bool isMate(MoveList<Us>& legalMoves) {
            if (legalMoves.size() == 0 && position.in_check<Us>()) {
            }
            return legalMoves.size() == 0 && position.in_check<Us>();
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
                cout << "start" << endl;
                evaluation = search<WHITE>(5, -64000, 64000);
                cout << "end" << endl;

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