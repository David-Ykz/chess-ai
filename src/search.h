using namespace std;

#include "chess-library/include/chess.hpp"
#include "evaluate.h"
#include "transposition_table.h"
#include <chrono>
#include <vector>

class Search {
private:
    Board board;
    Evaluator evaluator;
    Move rootMove;
    Move killerMoves[64][2];
    TranspositionTable transpositionTable;
    chrono::_V2::system_clock::time_point startTime;
    uint64_t timeLimitMicro;
    uint64_t nodesSearched;
    uint64_t numTranspositions;
    bool debug;

public:
    Search(Board& b, uint64_t t, bool d) : 
        board(b), 
        evaluator(), 
        timeLimitMicro(t), 
        transpositionTable(24),
        debug(d) {
        for (int i = 0; i < 64; i++) {
            killerMoves[i][0] = Move();
            killerMoves[i][1] = Move();
        }
    }

    inline vector<pair<int, const Move*>> orderCaptureMoves(Movelist& moves) {
        vector<pair<int, const Move*>> orderedMoves;
        for (const auto& move : moves) {
            int attacker = abs(evaluator.getPieceValue(board.at(move.from())));
            int victim = abs(evaluator.getPieceValue(board.at(move.to())));
            orderedMoves.emplace_back(victim * 100 - attacker, &move);
        }
        sort(orderedMoves.begin(), orderedMoves.end(), [](const auto& a, const auto& b)
        {
            return a.first > b.first;
        });
        return orderedMoves;
    }

    inline vector<pair<int, const Move*>> orderAllMoves(Movelist& moves, uint8_t ply, Move& ttMove) {
        vector<pair<int, const Move*>> orderedMoves;
        for (const auto& move : moves) {
            int moveScore = 0;
            if (move == ttMove) {
                moveScore = 1000000;
            } else if (board.at(move.to()) != Piece::NONE) {
                int attacker = abs(evaluator.getPieceValue(board.at(move.from())));
                int victim = abs(evaluator.getPieceValue(board.at(move.to())));
                moveScore = victim * 100 - attacker;
            } else if (killerMoves[ply][0] == move || killerMoves[ply][1] == move) {
                moveScore = 1000;
            }
            orderedMoves.emplace_back(moveScore, &move);
        }
        sort(orderedMoves.begin(), orderedMoves.end(), [](const auto& a, const auto& b)
        {
            return a.first > b.first;
        });
        return orderedMoves;
    };

    inline int quiescence(int alpha, int beta) {
        int score = evaluator.nnEvaluate(board);
        if (score >= beta) return beta;
        if (score > alpha) alpha = score;

        Movelist moves;
        movegen::legalmoves<movegen::MoveGenType::CAPTURE>(moves, board);
        vector<pair<int, const Move*>> orderedMoves = orderCaptureMoves(moves);
        for (int i = 0; i < orderedMoves.size(); ++i) {
            Move move = *orderedMoves[i].second;
            board.makeMove(move);
            int score = -quiescence(-beta, -alpha);
            board.unmakeMove(move);
            // Fail-hard framework
            if (score >= beta) return beta;
            if (score > alpha) alpha = score;
        }
        return alpha;
    }

    inline int negamax(uint8_t ply, uint8_t depth, int alpha, int beta) {
        if (depth == 0) return quiescence(alpha, beta);
        ++nodesSearched;
        if (ply <= 1) {
            auto endTime = chrono::high_resolution_clock::now();
            if (outOfTime(endTime)) return 0;
        }

        // Threefold repetition
        if (board.isRepetition(1)) return 0;

        // Transposition table lookup
        Move ttMove;
        TTEntry* entry = transpositionTable.probe(board.hash());
        if (entry != nullptr) {
            ttMove = entry->move;
            if (entry->depth >= depth) {
                int ttScore = entry->score;
                Bound bound = entry->bound;
                if (bound == EXACT) {
                    return ttScore;
                } else if (bound == LOWER_BOUND && ttScore >= beta) {
                    return ttScore;
                } else if (bound == UPPER_BOUND && ttScore <= alpha) {
                    return ttScore;
                }
            }
        }

        // Null move pruning
        bool inCheck = board.inCheck();
        if (!inCheck && depth >= 2) {
            board.makeNullMove();
            int score = -negamax(ply + 1, depth - 2 - depth/3, -beta, -beta + 1);
            board.unmakeNullMove();
            if (score >= beta) {
                transpositionTable.store(board.hash(), depth, score, LOWER_BOUND, Move());
                return score;
            }
            // if (score >= beta) return score;
        }
        int staticEval = evaluator.evaluate(board);
        // Reverse futility pruning
        if (depth <= 2 && entry == nullptr && !inCheck && staticEval >= beta + depth * 150) {
            transpositionTable.store(board.hash(), depth, staticEval, LOWER_BOUND, Move());
            return staticEval;
        }
        // if (depth <= 2 && entry == nullptr && !inCheck && staticEval >= beta + depth * 150) return staticEval;

        // Generate legal moves
        Movelist moves;
        movegen::legalmoves(moves, board);
        
        // Checkmate or stalemate
        if (moves.size() == 0) return board.inCheck() ? -(32000 - ply) : 0;

        int bestScore = -32000;
        int alphaOriginal = alpha;
        vector<pair<int, const Move*>> orderedMoves = orderAllMoves(moves, ply, ttMove);
        for (int i = 0; i < orderedMoves.size(); ++i) {
            // Futility pruning
            // if (depth < 10 && !inCheck && staticEval <= alpha - 500 - 256 * depth) {
            //     continue;
            // }

            Move move = *orderedMoves[i].second;
            board.makeMove(move);
            int score;
            // Late move reduction
            bool needsFullSearch = true;
            if (i > 3 && depth > 2) {
                needsFullSearch = false;
                score = -negamax(ply + 1, depth - 3, -alpha - 1, -alpha);
                if (alpha < score && score < beta) {
                    needsFullSearch = true;
                }
            }
            // PV Search
            if (needsFullSearch && i > 0) {
                score = -negamax(ply + 1, depth - 1, -alpha - 1, -alpha);
                needsFullSearch = alpha < score && score < beta;
            }
            if (needsFullSearch) {
                score = -negamax(ply + 1, depth - 1, -beta, -alpha);
            }
            board.unmakeMove(move);
            // Fail-soft framework
            if (score > bestScore) {
                bestScore = score;
                ttMove = move;
                if (ply == 0) rootMove = move;
                if (score > alpha) alpha = score;
                if (alpha >= beta) {
                    if (board.at(move.to()) == Piece::NONE && killerMoves[ply][0] != move) {
                        killerMoves[ply][1] = killerMoves[ply][0];
                        killerMoves[ply][0] = move;
                    }
                    break;
                }
            }
        }
        Bound bound;
        if (bestScore <= alphaOriginal) {
            bound = UPPER_BOUND;
        } else if (bestScore >= beta) {
            bound = LOWER_BOUND;
        } else {
            bound = EXACT;
        }
        ++numTranspositions;
        transpositionTable.store(board.hash(), depth, bestScore, bound, ttMove);
        return bestScore;
    }

    Move iterativeDeepening() {
        startTime = chrono::high_resolution_clock::now();
        int score;
        Move oldRootMove;
        for (int initalDepth = 1; initalDepth < 128; initalDepth++) {
            nodesSearched = 0;
            numTranspositions = 0;
            auto start = chrono::high_resolution_clock::now();
            // Aspiration windows
            bool needsFullSearch = false;
            if (initalDepth > 1) {
                for (int window = 4; window <= 1024; window <<= 2) {
                    int alpha = score - window;
                    int beta = score + window;
                    score = negamax(0, initalDepth, alpha, beta);
                    if (alpha <= score && score <= beta) {
                        break;
                    }
                }
                needsFullSearch = true;
            } else {
                needsFullSearch = true;
            }
            if (needsFullSearch) {
                score = negamax(0, initalDepth, -32000, 32000);
            }
            auto end = chrono::high_resolution_clock::now();
            if (outOfTime(end)) break;
            if (debug) {
                double duration = chrono::duration_cast<chrono::microseconds>(end - start).count() * 1.0 / 1000000;
                cout << "Depth: " << initalDepth << " | Score: " << score << " | Move: " << rootMove;
                cout << " | Time: " << duration << " | Nodes: " << nodesSearched << " | Transpositions: " << numTranspositions << endl;
            }
            oldRootMove = rootMove;
        }
        return oldRootMove;
    }

    bool outOfTime(chrono::_V2::system_clock::time_point end) {
        uint64_t durationMicro = chrono::duration_cast<chrono::microseconds>(end - startTime).count();
        return durationMicro > timeLimitMicro;
    }
};