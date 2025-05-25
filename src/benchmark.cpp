#include "search.h"

int main() {
//    chess::Board board = chess::Board("8/k7/3p4/p2P1p2/P2P1P2/8/8/K7 w - - ");
    chess::Board board = chess::Board();
    Search search(board, 1000000);
    search.iterativeDeepening();
    return 0;
}