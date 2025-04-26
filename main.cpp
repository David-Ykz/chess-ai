#include "chess_ai.h"

#include <iostream>

// git clone --branch hotfix-1 https://github.com/nkarve/surge.git
// g++ -O3 -march=znver3 -mtune=znver3 -flto -o main main.cpp ./surge/src/types.cpp ./surge/src/position.cpp ./surge/src/tables.cpp


//Computes the perft of the position for a given depth, using bulk-counting
//According to the https://www.chessprogramming.org/Perft site:
//Perft is a debugging function to walk the move generation tree of strictly legal moves to count 
//all the leaf nodes of a certain depth, which can be compared to predetermined values and used to isolate bugs
template<Color Us>
unsigned long long perft(Position& p, unsigned int depth) {
	//gk int nmoves;
	unsigned long long nodes = 0;

	MoveList<Us> list(p);

	if (depth == 1) return (unsigned long long) list.size();

	for (Move move : list) {
		p.play<Us>(move);
		nodes += perft<~Us>(p, depth - 1);
		p.undo<Us>(move);
	}

	return nodes;
}


void test_perft() {
	Position p;
	//gk Position::set("rnbqkbnr/pppppppp/8/8/8/8/PPPP1PPP/RNBQKBNR w KQkq -", p);
	Position::set("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -", p);
	std::cout << p;

	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
	auto n = perft<WHITE>(p, 6);
	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	auto diff = end - begin;

	std::cout << "Nodes: " << n << "\n";
	std::cout << "NPS: "
		<< int(n * 1000000.0 / std::chrono::duration_cast<std::chrono::microseconds>(diff).count())
		<< "\n";
	std::cout << "Time difference = "
		<< std::chrono::duration_cast<std::chrono::microseconds>(diff).count() << " [microseconds]\n";
}



int main() {
	//Make sure to initialise all databases before using the library!
	initialise_all_databases();
	zobrist::initialise_zobrist_keys();

	//gk call test_perft()
//	test_perft();

	Position p;
	// Position::set("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -", p);
	Position::set("r4k1r/pp3ppp/1qp1p3/4Nb2/2PP2n1/1Q4P1/P3PPBP/R4RK1 w - -", p);
//	Position::set("8/k7/3p4/p2P1p2/P2P1P2/8/8/K7 w - -", p);
//	Position::set("r3r1k1/ppq1bpp1/3p4/2p1nbP1/8/P1NP4/BPP1Q3/R2K3R w - -", p);
// Mate in 2
//	Position::set("r1b1qk1r/1p1np1b1/p4nQ1/3p2Np/2pP1B1P/2P1P3/PP1N1PP1/R3K2R w KQ -", p);
// Mate in 4
//	Position::set("8/2p2pk1/p1p2N1p/4P3/2r5/4R3/r7/6K1 w - -", p);
//	Position::set("6k1/2qr1p2/r5p1/p3p1QR/b1pbP3/BP6/2P4P/1K3R2 w - -", p);
//	Position::set("1k6/8/1K6/8/8/8/8/5R2 w - -", p);
//	Position::set("8/8/8/8/8/2k5/1q6/K7 w - -", p);

	std::cout << p;


 	ChessAI ai = ChessAI(p);
	ai.makeMove();

	return 0;
}


// create match manager
// implement transposition table
// add iterative deepening
// add basic pst?
// figure out how to pass board into ai
// figure out training data
// pv table?
