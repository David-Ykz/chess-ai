using namespace std;

#pragma once

#include "surge/position.h"
#include "surge/tables.h"
#include "surge/types.h"

#include "evaluation.h"

#include <chrono>

class SearchResult {
public:
    Move move;
    int evaluation;
    int depthSearched;
    int nodesSearched;
    int timeTakenMicro;
    double MNodesPerSecond;
    SearchResult(Move move, int evaluation, int depthSearched, int nodesSearched, int timeTakenMicro) :
        move(move), evaluation(evaluation), depthSearched(depthSearched), nodesSearched(nodesSearched), timeTakenMicro(timeTakenMicro)
        {
            MNodesPerSecond = static_cast<double>(nodesSearched) / timeTakenMicro;
        }
    friend ostream& operator<< (ostream& os, const SearchResult& result) {
        os << "Move: " << result.move;
        os << " | Eval: " << result.evaluation;
        os << " | Depth: " << result.depthSearched;
        os << " | Nodes: " << result.nodesSearched;
        os << " | Time: " << result.timeTakenMicro / 1000000.0;
        os << " | MNPS: " << result.MNodesPerSecond;
        os << endl;
        return os;
    }

};


class Search {
private:
    Evaluation evaluator;
    Position position;
    Move bestMove;
    int nodesSearched;
    int timeTakenMicro;
    const int CHECKMATE_SCORE = 64000;

public:
    Search(Position& p) : position(p) {
        initialise_all_databases();
        zobrist::initialise_zobrist_keys();    
        evaluator.initialize("nn/nn-1111cefa1111.nnue");
    }

    template <Color Us>
    int negamax(int ply, int depth);
    
    template <Color Us>
    SearchResult search();

};

