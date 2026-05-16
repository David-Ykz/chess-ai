#include "../include/search.h"
#include <iostream>
#include <chrono>
using namespace chess;
using namespace std;

uint64_t perft(Board& board, int depth) {
    Movelist moves;
    movegen::legalmoves(moves, board);

    if (depth == 1) {
        return moves.size();
    }

    uint64_t nodes = 0;

    for (int i = 0; i < moves.size(); i++) {
        const auto move = moves[i];
        board.makeMove(move);
        nodes += perft(board, depth - 1);
        board.unmakeMove(move);
    }

    return nodes;
}

int main () {
    Board board = Board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    auto t1 = chrono::high_resolution_clock::now();
    uint64_t nodes = perft(board, 6);
    auto t2 = chrono::high_resolution_clock::now();
    uint64_t t_micro = chrono::duration_cast<chrono::microseconds>(t2 - t1).count();
    int nps = nodes * 1000000/t_micro;
    cout << "Nodes: " << nodes << " | Time: " << t_micro * 1.0 / 1000000 << " | NPS: " << nps << endl;

    return 0;
}