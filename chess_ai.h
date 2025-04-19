#pragma once
#include "eval_move.h"
#include <limits>
#include <algorithm>

class ChessAI {
    private:
        Position& position;

        int numMinimaxSearches = 0;
        int numQuiescenceSearches = 0;
        int numPruned = 0;

        int depthToSearch = 5;

        const int CHECKMATE_SCORE = 10000;
        int PIECE_VALUES[14] = {100, 300, 300, 500, 900, 0, 0, 0, -100, -300, -300, -500, -900, 0};
    public:
        ChessAI(Position& p) : position(p) {}
        void printDebug() {
            cout << "Number of minimax searches: " << numMinimaxSearches << endl;
            cout << "Number of quiescence searches: " << numQuiescenceSearches << endl;
            cout << "Number of nodes pruned: " << numPruned << endl;
        }

        template <Color Us>
        vector<EvalMove> orderMoves(MoveList<Us>& legalMoves, bool filterCaptures) {
            vector<EvalMove> orderedMoves;
            for (Move move : legalMoves) {
                int moveScore = 0;
                if (move.is_capture()) {
                    Piece from = position.at(move.from());
                    Piece to = position.at(move.to());
                    moveScore = abs(PIECE_VALUES[to]) - abs(PIECE_VALUES[from])/10;
                    EvalMove evalMove = EvalMove(move, 0, moveScore);
                    orderedMoves.push_back(evalMove);
                } else if (!filterCaptures) {
                    EvalMove evalMove = EvalMove(move, 0, moveScore);
                    orderedMoves.push_back(evalMove);
                }
            }
            sort(orderedMoves.begin(), orderedMoves.end());
            return orderedMoves;
        }

        template <Color Us>
        EvalMove minimax(int depth, int alpha, int beta) {
            ++numMinimaxSearches;
            MoveList<Us> legalMoves(position);
            if (isDraw<Us>(legalMoves)) {
                return EvalMove(0);
            }
            if (isMate<Us>(legalMoves)) {
                constexpr int sign = (Us == Color::WHITE) ? 1 : -1;
                return (CHECKMATE_SCORE - depthToSearch) * -sign;
            }
        
            if (depth == 0) {
                return quiescenceSearch<Us>(alpha, beta);
            }

            constexpr int startingEval = (Us == WHITE) ? numeric_limits<int>::min() : numeric_limits<int>::max();
            EvalMove bestMove = EvalMove(startingEval);
            vector<EvalMove> orderedMoves = orderMoves(legalMoves, false);
            for (EvalMove evalMove : orderedMoves) {
                position.play<Us>(evalMove.move);
                evalMove.evaluation = minimax<~Us>(depth - 1, alpha, beta).evaluation;
                position.undo<Us>(evalMove.move);
            
                if constexpr (Us == WHITE) {
                    if (evalMove.evaluation > bestMove.evaluation) {
                        bestMove = evalMove;
                    }
                    alpha = max(alpha, evalMove.evaluation);
                } else {
                    if (evalMove.evaluation < bestMove.evaluation) {
                        bestMove = evalMove;
                    }
                    beta = min(beta, evalMove.evaluation);
                }
                if (beta <= alpha) {
                    ++numPruned;
                    return bestMove;
                }
            }
            return bestMove;        
        }
        template <Color Us>
        EvalMove quiescenceSearch(int alpha, int beta) {
            ++numQuiescenceSearches;
            EvalMove bestMove = EvalMove(evaluate());

            if constexpr (Us == WHITE) {
                alpha = max(alpha, bestMove.evaluation);
            } else {
                beta = min(beta, bestMove.evaluation);
            }

            if (beta <= alpha) {
                ++numPruned;
                return bestMove;
            }

            MoveList<Us> legalMoves(position);

            vector<EvalMove> orderedMoves = orderMoves(legalMoves, true);
            for (EvalMove evalMove : orderedMoves) {
                position.play<Us>(evalMove.move);
                evalMove.evaluation = quiescenceSearch<~Us>(alpha, beta).evaluation;
                position.undo<Us>(evalMove.move);
            
                if constexpr (Us == WHITE) {
                    if (evalMove.evaluation > bestMove.evaluation) {
                        bestMove = evalMove;
                    }
                    alpha = max(alpha, evalMove.evaluation);
                } else {
                    if (evalMove.evaluation < bestMove.evaluation) {
                        bestMove = evalMove;
                    }
                    beta = min(beta, evalMove.evaluation);
                }
                if (beta <= alpha) {
                    ++numPruned;
                    return bestMove;
                }
            }
            return bestMove;        
        }
        template <Color Us>
        bool isDraw(MoveList<Us>& legalMoves) {
            return legalMoves.size() == 0;
        }
        template <Color Us>
        bool isMate(MoveList<Us>& legalMoves) {
            return legalMoves.size() == 0 && position.in_check<Us>();
        }
        int evaluate() {
            int evaluation = 0;
            for (int i = WHITE_PAWN; i < NO_PIECE; ++i) {
                Piece piece = static_cast<Piece>(i);
                evaluation += pop_count(position.bitboard_of(piece)) * PIECE_VALUES[i];
            }
            // evaluation += pop_count(position.bitboard_of(WHITE_PAWN)) * PAWN_VALUE;
            // evaluation += pop_count(position.bitboard_of(WHITE_KNIGHT)) * KNIGHT_VALUE;
            // evaluation += pop_count(position.bitboard_of(WHITE_BISHOP)) * BISHOP_VALUE;
            // evaluation += pop_count(position.bitboard_of(WHITE_ROOK)) * ROOK_VALUE;
            // evaluation += pop_count(position.bitboard_of(WHITE_QUEEN)) * QUEEN_VALUE;
        
            // evaluation += pop_count(position.bitboard_of(BLACK_PAWN)) * -PAWN_VALUE;
            // evaluation += pop_count(position.bitboard_of(BLACK_KNIGHT)) * -KNIGHT_VALUE;
            // evaluation += pop_count(position.bitboard_of(BLACK_BISHOP)) * -BISHOP_VALUE;
            // evaluation += pop_count(position.bitboard_of(BLACK_ROOK)) * -ROOK_VALUE;
            // evaluation += pop_count(position.bitboard_of(BLACK_QUEEN)) * -QUEEN_VALUE;
        
            return evaluation;
        }

};