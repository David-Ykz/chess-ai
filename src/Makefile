main:
	g++ -O3 -flto -mavx2 -march=znver3 -mtune=znver3 -o main main.cpp


old-main:
	g++ -O3 -flto -mavx2 -march=znver3 -mtune=znver3 -o main main.cpp ./surge/types.cpp ./surge/position.cpp ./surge/tables.cpp

benchmark:
	g++ -O3 -flto -mavx2 -march=znver3 -mtune=znver3 -o b benchmark.cpp ./nn/probe.cpp ./nn/evaluate.cpp ./nn/misc.cpp ./nn/position.cpp ./nn/bitboard.cpp ./nn/nnue/evaluate_nnue.cpp ./nn/nnue/features/half_ka_v2_hm.cpp

uci:
	g++ -O3 -flto -mavx2 -march=znver3 -mtune=znver3 -o engine uci_interface.cpp ./nn/probe.cpp ./nn/evaluate.cpp ./nn/misc.cpp ./nn/position.cpp ./nn/bitboard.cpp ./nn/nnue/evaluate_nnue.cpp ./nn/nnue/features/half_ka_v2_hm.cpp
