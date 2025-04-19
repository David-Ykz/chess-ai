#include "chess_ai.h"

#include <iostream>
#include <chrono>

// g++ -O3 -march=znver3 -mtune=znver3 -flto -o main main.cpp ./surge/src/types.cpp ./surge/src/position.cpp ./surge/src/tables.cpp chess_ai.cpp


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
//	Position::set("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -", p);
	Position::set("r4k1r/pp3ppp/1qp1p3/4Nb2/2PP2n1/1Q4P1/P3PPBP/R4RK1 w - -", p);
	std::cout << p;
	ChessAI ai = ChessAI(p);
	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
	EvalMove evalMove = ai.minimax<WHITE>(5, numeric_limits<int>::min(), numeric_limits<int>::max());
	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	auto diff = end - begin;
	cout << evalMove.move << endl;
	ai.printDebug();
	cout << "Time difference = "
		<< chrono::duration_cast<std::chrono::microseconds>(diff).count() << " [microseconds]\n";
	return 0;
}



