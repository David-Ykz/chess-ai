#include "chess_ai.h"

#include <iostream>

// g++ -O3 -march=znver3 -mtune=znver3 -flto -o positiontester positiontester.cpp ./surge/src/types.cpp ./surge/src/position.cpp ./surge/src/tables.cpp

int main() {
	initialise_all_databases();
	zobrist::initialise_zobrist_keys();

    string gameFens[6] = {
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -",
        "r4k1r/pp3ppp/1qp1p3/4Nb2/2PP2n1/1Q4P1/P3PPBP/R4RK1 w - -",
        "r2q1rk1/pp4pp/2p1p3/3pp1bB/2nPP3/P2Q2P1/1PP1NP1P/3RR1K1 w - -",
        "r2q1rk1/1p3pp1/2p1p2p/p2p1b2/PbnP4/2N2NPP/1PPQPPB1/2R1R1K1 w - -",
        "5q2/p4k2/1pQ1pn2/1P1n1pB1/P2P3P/5R2/5P2/6K1 w -  -",
        "rn2k2r/pp3pp1/4pn1p/1P1q4/P2P4/n4N2/3B1PPP/R2Q1RK w kq -"
    };

    for (int i = 0; i < 6; i++) {
        Position p;
        Position::set(gameFens[i], p);
        ChessAI ai = ChessAI(p);
        p.play<WHITE>(ai.findMove<WHITE>());
    }

	return 0;
}

