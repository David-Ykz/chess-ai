#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include "search.h"

static Board board = Board();

void positionCommand(istringstream& iss) {
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

void goCommand() {
    Search searcher = Search(board, 5000000, true);
    Move bestMove = searcher.iterativeDeepening();
    std::cout << "bestmove " << uci::moveToUci(bestMove) << std::endl;
    board.makeMove(bestMove);
}

void uciLoop() {
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
            positionCommand(iss);
        } else if (command == "go") {
            goCommand();
        } else if (command == "lazy") {
            string token;
            iss >> token;
            board.makeMove(uci::uciToMove(board, token));
            goCommand();
        } else if (command == "quit") {
            break;
        }
    }
}

int main() {
    uciLoop();
}