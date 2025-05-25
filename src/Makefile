main:
	g++ -O3 -flto -mavx2 -march=znver3 -mtune=znver3 -o main main.cpp


old-main:
	g++ -O3 -flto -mavx2 -march=znver3 -mtune=znver3 -o main main.cpp ./surge/types.cpp ./surge/position.cpp ./surge/tables.cpp

benchmark:
	g++ -O3 -flto -mavx2 -march=znver3 -mtune=znver3 -o b benchmark.cpp