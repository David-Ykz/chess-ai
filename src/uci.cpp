#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include "../include/search.h"


void positionCommand(Board &board, istringstream &iss) {
    std::string token, fen;
    iss >> token; // "startpos"
    board = Board();
    iss >> token; // "move"
    while (iss >> token) {
        Move move = uci::uciToMove(board, token);
        if (move != Move::NO_MOVE)
            board.makeMove(move);
    }
}

void goCommand(Board& board, std::istringstream& iss) {
    int wtime, btime, winc, binc;
    int movestogo;
    int movetime;

    std::string token;
    while (iss >> token) {
        if (token == "wtime") iss >> wtime;
        else if (token == "btime") iss >> btime;
        else if (token == "winc") iss >> winc;
        else if (token == "binc") iss >> binc;
        // else if (token == "movestogo") iss >> movestogo;
        else if (token == "movetime") iss >> movetime;
        else if (token == "infinite") movetime = 1000;
    }

    // Determine side to move
    bool whiteToMove = board.sideToMove() == Color::WHITE;

    int timeLeft = whiteToMove ? wtime : btime;
    int increment = whiteToMove ? winc : binc;

    int allottedTime;
    if (movetime > 0) {
        allottedTime = movetime;
    } else {
        allottedTime = max(timeLeft / 20 + increment / 2, increment);
    }

    Search searcher(board, allottedTime);
    SearchResult result = searcher.search();

    cout << "bestmove " << uci::moveToUci(result.bestMove) << endl;
}

void uciLoop() {
    Board board;
    std::string line;
    while (std::getline(std::cin, line)) {
        std::istringstream iss(line);
        std::string command;
        iss >> command;

        if (command == "uci") {
            cout << "uciok\n";
        } else if (command == "isready") {
            cout << "readyok\n";
        } else if (command == "ucinewgame") {
            board = Board();
        } else if (command == "position") {
            positionCommand(board, iss);
        } else if (command == "go") {
            goCommand(board, iss);
        } else if (command == "quit") {
            break;
        }
    }
}

int main() {
    uciLoop();
}


// <Saruman(37): info depth 7 nodes 11180 nps 4088474 pv d5d6 e8e7 f2g2 g7f6 g2f2 f6g6 f2g2  score cp -790