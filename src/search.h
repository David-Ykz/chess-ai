using namespace std;

#include "chess-library/include/chess.hpp"
#include "evaluate.h"
#include <chrono>
#include <vector>

class Search {
private:
    Board board;
    Evaluator evaluator;
    Move rootMove;
    Move killerMoves[64][2];
    chrono::_V2::system_clock::time_point startTime;
    uint64_t timeLimitMicro;
    uint64_t nodesSearched;

public:
    Search(Board b, uint64_t t) : board(b), evaluator(), timeLimitMicro(t) {
        for (int i = 0; i < 64; i++) {
            killerMoves[i][0] = Move();
            killerMoves[i][1] = Move();
        }
    }

    inline vector<pair<int, const Move*>> orderMoves(Movelist& moves, uint8_t ply) {
        vector<pair<int, const Move*>> orderedMoves;
        for (const auto& move : moves) {
            int moveScore = 0;
            if (board.at(move.to()) != Piece::NONE) {
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

    int quiescence(int alpha, int beta) {
        int score = evaluator.evaluate(board);
        if (score >= beta) return beta;
        if (score > alpha) alpha = score;

        Movelist moves;
        movegen::legalmoves<movegen::MoveGenType::CAPTURE>(moves, board);
        vector<pair<int, const Move*>> orderedMoves = orderMoves(moves, 0);
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

    int negamax(uint8_t ply, uint8_t depth, int alpha, int beta) {
        ++nodesSearched;

        if (depth == 0) {
            return quiescence(alpha, beta);
        }

        if (ply <= 1) {
            auto endTime = chrono::high_resolution_clock::now();
            if (outOfTime(endTime)) return 0;
        }

        // Threefold repetition
        if (board.isRepetition(1)) return 0;

        // Generate legal moves
        Movelist moves;
        movegen::legalmoves(moves, board);
        
        // Checkmate or stalemate
        if (moves.size() == 0) return board.inCheck() ? -(32000 - ply) : 0;

        // Null move pruning
        bool inCheck = board.inCheck();
        if (!inCheck && depth >= 2) {
            board.makeNullMove();
            int score = -negamax(ply + 1, depth - 2, -beta, -beta + 1);
            board.unmakeNullMove();
            if (score >= beta) return score;
        }

        int bestScore = -32000;
        vector<pair<int, const Move*>> orderedMoves = orderMoves(moves, ply);
        for (int i = 0; i < orderedMoves.size(); ++i) {
            Move move = *orderedMoves[i].second;
            board.makeMove(move);
            int score = -negamax(ply + 1, depth - 1, -beta, -alpha);
            board.unmakeMove(move);
            // Fail-soft framework
            if (score > bestScore) {
                bestScore = score;
                if (ply == 0) rootMove = move;
                if (score > alpha) alpha = score;
                if (score >= beta) {
                    if (board.at(move.to()) == Piece::NONE && killerMoves[ply][0] != move) {
                        killerMoves[ply][1] = killerMoves[ply][0];
                        killerMoves[ply][0] = move;
                    }
                    break;
                }
            }
        }
        return bestScore;
    }

    Move iterativeDeepening() {
        startTime = chrono::high_resolution_clock::now();
        for (int initalDepth = 1; initalDepth < 128; initalDepth++) {
            nodesSearched = 0;
            auto start = chrono::high_resolution_clock::now();
            int score = negamax(0, initalDepth, -32000, 32000);
            auto end = chrono::high_resolution_clock::now();
            if (outOfTime(end)) break;
            double duration = chrono::duration_cast<chrono::microseconds>(end - start).count() * 1.0 / 1000000;
            cout << "Depth: " << initalDepth << " | Score: " << score << " | Move: " << rootMove;
            cout << " | Time taken: " << duration << " | Nodes searched: " << nodesSearched << endl;
        }
        return rootMove;
    }

    bool outOfTime(chrono::_V2::system_clock::time_point end) {
        uint64_t durationMicro = chrono::duration_cast<chrono::microseconds>(end - startTime).count();
        return durationMicro > timeLimitMicro;
    }
};