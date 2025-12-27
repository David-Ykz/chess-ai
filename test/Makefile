CFLAGS := -std=c++17 -O3 -march=native -flto

perft:
	g++ $(CFLAGS) -o ../bin/perft perft.cpp
	../bin/perft

unit:
	g++ $(CFLAGS) -o ../bin/unit unit.cpp ../src/search.cpp
	../bin/unit