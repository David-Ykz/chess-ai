#include "search.h"

int main() {
    chess::Board board = chess::Board("r1bq1rk1/1p1nbpp1/p3p2p/3pP3/2p2B2/2N1P3/PPPQBPPP/2R2RK1 w - - ");
    // chess::Board board = chess::Board();
    Search search(board, 3000000, true);
    search.iterativeDeepening();
    return 0;
}