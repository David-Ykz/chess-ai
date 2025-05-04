#include "search.h"

int main() {
    string positions[] = {
        "8/8/8/8/6p1/8/8/1k1K4 b - -",
        "8/8/1p1k3p/1Pp2p2/2P2P2/3K3P/8/8 w - -",
        "5rk1/6pp/p6n/1p6/2r5/5N1P/6PK/R3R3 w - -",
        "rnbqkb1r/pp3ppp/5n2/8/3pN3/1Q3PP1/PP1PP1BP/R1B1K1NR b KQkq -",
        "r5k1/2pq2pp/4b3/pp1pP3/3P4/1PPQ4/6PP/R1B3K1 b q -",
        "r4rk1/ppq1b1pn/2pp3p/2n5/2PN1B2/P1Nb2PP/1P1Q2BK/R4R2 w - -",
        "8/5k2/5npp/1B1p4/4p1P1/4P2P/r4P2/2R3K1 b - -",
        "r7/2P5/5p2/3k1Ppp/3P4/4K3/p4P2/2R5 w - -",
        "8/5p2/1p3kp1/pB5p/7P/1Pb1P1P1/5P2/3K4 b - -",
        "6R1/5k2/5p2/4p3/8/5PK1/1r6/8 w - -",
        "8/3k3p/5p2/1KPb4/6p1/1p4P1/1P1B1P1P/8 w - -",
        "R7/8/6pk/P3rp2/7P/3K4/8/8 b - -"
    };
    int evaluations[] = {0, 0, 0, 41, -36, -7, 0, 0, 0, -9, 73, 0};

    for (int i = 0; i < 12; i++) {
        Position p;
        Position::set(positions[i], p);
        Search s = Search(p);
        if (p.turn() == WHITE) {
            cout << s.search<WHITE>();
        } else {
            cout << s.search<BLACK>();
        }
    }

    return 0;
}