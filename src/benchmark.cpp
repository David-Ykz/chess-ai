#include "search.h"

int main() {
    chess::Board board = chess::Board();
    Search search(board, 1000000);
    search.iterativeDeepening();
    return 0;
}