CFLAGS := -std=c++17 -O3 -march=native -flto

perft:
	g++ $(CFLAGS) -o bin/perft test/perft.cpp
	bin/perft

unit:
	g++ $(CFLAGS) -o bin/unit test/unit.cpp src/chess.cpp src/search.cpp src/nnue.cpp
	bin/unit

uci:
	g++ $(CFLAGS) -o bin/uci src/uci.cpp src/chess.cpp src/search.cpp src/nnue.cpp
