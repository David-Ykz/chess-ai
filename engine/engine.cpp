#include "chess_ai.h"
#include <iostream>

// g++ -O3 -march=znver3 -mtune=znver3 -flto -o engine engine.cpp ./surge/src/types.cpp ./surge/src/position.cpp ./surge/src/tables.cpp

int main() {
	initialise_all_databases();
	zobrist::initialise_zobrist_keys();

    Position p;
    string fen;
    getline(cin, fen);
		Position::set(fen, p);
		ChessAI ai = ChessAI(p);
        if (p.turn() == WHITE) {
            auto candidateMoves = ai.generateCandidateMoves<WHITE>();
            for (const auto& move : candidateMoves) {
                cout << move.first << "|" << move.second << ",";
            }
            cout << endl;
        } else {
            cout << ai.findMove<BLACK>() << endl;
        }

	return 0;
}