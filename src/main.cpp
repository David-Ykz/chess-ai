#include "chess-library/include/chess.hpp"
#include <iostream>
#include <chrono>


uint64_t perft(int depth, chess::Board& board) {
    chess::Movelist moves;
    chess::movegen::legalmoves(moves, board);

    if (depth == 1) {
        return moves.size();
    }

    uint64_t nodes = 0;

    for (const auto& move : moves) {
        board.makeMove(move);
        nodes += perft(depth - 1, board);
        board.unmakeMove(move);
    }

    return nodes;

}


int main() {
    std::cout << "Hello World!" << std::endl;
    chess::Board board = chess::Board();
    auto t1 = std::chrono::high_resolution_clock::now();
    uint64_t nodes = perft(6, board);
    auto t2 = std::chrono::high_resolution_clock::now();
    uint64_t t_micro = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
    int nps = nodes * 100000/t_micro;
    std::cout << nodes << " " << t_micro * 1.0 / 1000000 << " " << nps << std::endl;
    auto start = std::chrono::high_resolution_clock::now();
    auto end = std::chrono::high_resolution_clock::now();
    bool endloop = false;
    uint64_t num = 0;
    while (!endloop) {
//    while (std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() < 1000000) {
        ++num;
        if (num % 1000 == 0) {
            end = std::chrono::high_resolution_clock::now();
            endloop = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() >= 1000000;
        }
    }
    std::cout << num << std::endl;
}