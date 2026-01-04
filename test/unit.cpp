#include "../include/search.h"
#include <iostream>
#include <chrono>
using namespace std;
#define NUM_TESTS 20

struct TestCase {
    string fen;
    uint64_t allottedTime;
    int expectedEval;
    Square expectedFrom;
    Square expectedTo;
};

bool runTest(string fen, uint64_t allottedTime, int expectedEval, Square expectedFrom, Square expectedTo) {
    cout << "Fen: " << fen << endl;
    TranspositionTable tt;
    Search searcher = Search(fen, allottedTime, tt);
    SearchResult result = searcher.search();
    Square from = result.bestMove.from();
    Square to = result.bestMove.to();
    if (abs(result.eval - expectedEval) > 100) {
        cout << "FAIL - Expected: " << expectedFrom << expectedTo << ", " << expectedEval << " | Actual: " << from << to << ", " << result.eval << endl;
        return false;
    }
    if (abs(expectedEval) > 200) {
        if (from != expectedFrom || to != expectedTo) {
            cout << "FAIL - Expected: " << expectedFrom << expectedTo << ", " << expectedEval << " | Actual: " << from << to << ", " << result.eval << endl;
            return false;
        }
    }
    cout << "PASS" << endl;
    return true;
}

TestCase testCases[NUM_TESTS] = {
    {"6k1/5ppp/6q1/8/8/8/5PPP/R5K1 w - - 0 1", 100, 98999, Square("a1"), Square("a8")},
    {"1kr4r/1pq2ppp/p2bp3/3p4/N1nPn3/1Q2PN2/PP3PBP/1KR1B2R b - - 17 21", 100, 98997, Square("c4"), Square("a3")},
    {"r1b1R3/ppp3kp/5rp1/n4p2/3q1P2/1Q6/P5PP/R6K w - - 2 24", 100, 98997, Square("e8"), Square("g8")},
    {"5rk1/4pp2/p5pQ/8/r2Nb1P1/q1P5/P7/K2R1R2 w - - 3 26", 4000, 98991, Square("h6"), Square("f8")},
    {"r3k3/8/8/8/8/8/R7/4K3 b - - 0 1", 100, 500, Square("a8"), Square("a2")},
    {"rnb3k1/ppp3pp/4p3/3p4/3PPR1q/3B2n1/PPP3P1/RNBQ2K1 b - - 4 13", 100, 400, Square("h4"), Square("h1")},
    {"r1b1r1k1/pp2np2/3q2p1/2p1B2p/3n3P/P2P1NP1/1P2PPB1/1R1QK2R b K - 4 15", 100, 200, Square("d4"), Square("f3")},
    {"3n1n2/1p2rpk1/p5p1/2PP3p/PP5P/3Q1qP1/5P2/1R1R2K1 b - - 2 30", 500, 300, Square("e7"), Square("e1")},
    {"7k/1p4b1/pQ5P/5b2/4p3/1P6/P3PP1P/2Bq1BK1 b - - 0 29", 500, 400, Square("f5"), Square("h3")},
    {"8/p5k1/2p1rp2/5p1p/P2Pq3/5N1P/1R2nPPK/4Q3 b - - 11 34", 500, 300, Square("e4"), Square("f4")},
    {"rn1q1rk1/pp2bn1p/4Q1pP/4Np2/3p4/3B2P1/PPP2P2/R1B1K2R b KQ - 2 15", 100, 200, Square("d8"), Square("a5")},
    {"2r1k1r1/pp3p1Q/4b3/q7/2P5/2P5/P4PPP/1K1R1B1R b - - 0 19", 500, 500, Square("e6"), Square("f5")},
    {"1k1b4/8/2bp4/pp1N1p1p/2P1rPp1/1P2R3/P5PP/5R1K b - - 1 34", 500, 100, Square("c6"), Square("d5")},
    {"2kr3r/2p1q2p/6p1/5p2/Q2b1P2/4P3/PPPB3P/R3R2K b - - 3 25", 500, 500, Square("e7"), Square("e4")},
    {"2r5/8/7k/4N1p1/5p1p/P4R1P/5RPK/r7 b - - 5 46", 500, 200, Square("c8"), Square("c1")},
};

TestCase NNUETestCases[NUM_TESTS] = {
    {"rn1qr1k1/1bp2ppp/p2b1n2/1P1pN3/3P4/1P2B1P1/4PPBP/RN1Q1RK1 b - - 0 12", 100, 20, Square("a1"), Square("a2")},
    {"7k/8/7K/8/8/8/8/4R3 w - - 0 1", 100, 31999, Square("e1"), Square("e8")},
    {"6k1/5ppp/6q1/8/8/8/5PPP/R5K1 w - - 0 1", 100, 31999, Square("a1"), Square("a8")},
    {"1kr4r/1pq2ppp/p2bp3/3p4/N1nPn3/1Q2PN2/PP3PBP/1KR1B2R b - - 17 21", 100, 31997, Square("c4"), Square("a3")},
    {"r1b1R3/ppp3kp/5rp1/n4p2/3q1P2/1Q6/P5PP/R6K w - - 2 24", 100, 31997, Square("e8"), Square("g8")},
    {"5rk1/4pp2/p5pQ/8/r2Nb1P1/q1P5/P7/K2R1R2 w - - 3 26", 100, 31991, Square("h6"), Square("f8")},
    {"r3k3/8/8/8/8/8/R7/4K3 b - - 0 1", 100, 975, Square("a8"), Square("a2")},
    {"rnb3k1/ppp3pp/4p3/3p4/3PPR1q/3B2n1/PPP3P1/RNBQ2K1 b - - 4 13", 100, 1200, Square("h4"), Square("h1")},
    {"r1b1r1k1/pp2np2/3q2p1/2p1B2p/3n3P/P2P1NP1/1P2PPB1/1R1QK2R b K - 4 15", 100, 500, Square("d4"), Square("f3")},
    {"3n1n2/1p2rpk1/p5p1/2PP3p/PP5P/3Q1qP1/5P2/1R1R2K1 b - - 2 30", 100, 650, Square("e7"), Square("e1")},
    {"7k/1p4b1/pQ5P/5b2/4p3/1P6/P3PP1P/2Bq1BK1 b - - 0 29", 200, 950, Square("f5"), Square("h3")},
    {"8/p5k1/2p1rp2/5p1p/P2Pq3/5N1P/1R2nPPK/4Q3 b - - 11 34", 500, 640, Square("e4"), Square("f4")},
    {"rn1q1rk1/pp2bn1p/4Q1pP/4Np2/3p4/3B2P1/PPP2P2/R1B1K2R b KQ - 2 15", 100, 400, Square("d8"), Square("a5")},
    {"2r1k1r1/pp3p1Q/4b3/q7/2P5/2P5/P4PPP/1K1R1B1R b - - 0 19", 100, 1000, Square("e6"), Square("f5")},
    {"1k1b4/8/2bp4/pp1N1p1p/2P1rPp1/1P2R3/P5PP/5R1K b - - 1 34", 100, 600, Square("c6"), Square("d5")},
    {"2kr3r/2p1q2p/6p1/5p2/Q2b1P2/4P3/PPPB3P/R3R2K b - - 3 25", 100, 1200, Square("e7"), Square("e4")},
    {"2r5/8/7k/4N1p1/5p1p/P4R1P/5RPK/r7 b - - 5 46", 100, 650, Square("c8"), Square("c1")},
    {"3r2k1/Q2P1p2/1p4pp/5q2/7P/P1p3P1/5P2/3R2K1 b - - 0 32", 100, 600, Square("c3"), Square("c2")},
    {"4K3/5P2/8/1k6/3pP3/R1r5/P7/8 b - - 0 56", 500, 460, Square("c3"), Square("a3")},  
    // {"8/k7/3p4/p2P1p2/P2P1P2/8/8/K7 w - - ", 1000, 900, Square("a1"), Square("b1")},
    {"r1bqkb1r/1ppp1ppp/p1n2n2/4p3/B3P3/5N2/PPPP1PPP/RNBQK2R w KQkq - 2 5", 100, 30, Square("e1"), Square("g1")},
};

int main () {
    int numTestsPass = 0;
    for (int i = 0; i < NUM_TESTS; i++) {
        TestCase tc = NNUETestCases[i];
        if (runTest(tc.fen, tc.allottedTime, tc.expectedEval, tc.expectedFrom, tc.expectedTo)) {
            numTestsPass++;
        }
    }

    cout << numTestsPass << "/" << NUM_TESTS << endl;
    return 0;
}