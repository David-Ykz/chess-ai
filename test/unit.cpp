#include "../include/search.h"
#include <iostream>
#include <chrono>
using namespace std;

int main () {
    Board board = Board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    Search searcher = Search(board, 1000);
    SearchResult result = searcher.search();
    cout << result << endl;

    return 0;
}