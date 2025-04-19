#pragma once

using namespace std;

#include "./surge/src/types.h"
#include "./surge/src/position.h"
#include "./surge/src/tables.h"
#include <memory>

class EvalMove {
    public:
        Move move;
        int evaluation;
        int score;
        EvalMove(int e) : evaluation(e) {}
        EvalMove(Move m, int e, int s) : move(m), evaluation(e), score(s) {}
        EvalMove& operator=(const EvalMove&) = default;
        bool operator<(const EvalMove& e) const {
            return score < e.score;
        }
};