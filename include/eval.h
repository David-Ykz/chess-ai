#include "chess.hpp"
#include "tables.h"
#include "nnue.h"
using namespace chess;

class Eval {
private:
    int midgamePieceTotals[12][64];
    int endgamePieceTotals[12][64];
    void initializePSTWeights() {
        for (int p = 0U; p <= 11U; p++) {
            for (int i = 0; i < 64; i++) {
                midgamePieceTotals[p][i] = midgamePieceValues[p] + midgamePst[p][i];
                endgamePieceTotals[p][i] = endgamePieceValues[p] + endgamePst[p][i];
            }
        }
    }

public:
    NNUE::Net nnue;

    Eval() {
        initializePSTWeights();
    }

    inline int basicEval(Board &board) {
        int eval = 0;
        Bitboard occ = board.occ();
        // Get piece score for each piece on the board
        while (occ) {
            uint8_t square = occ.pop();
            uint8_t pieceType = board.at(square);
            eval += endgamePieceValues[pieceType];
        }
        return board.sideToMove() == Color::WHITE ? eval : -eval;
    }

    inline int pstEval(Board &board) {
        int midgameEvaluation = 0;
        int endgameEvaluation = 0;
        int gamePhase = 0;
        Bitboard occ = board.occ();
        // Get piece score for each piece on the board
        while (occ) {
            uint8_t square = occ.pop();
            uint8_t pieceType = board.at(square);
            midgameEvaluation += midgamePieceTotals[pieceType][square];
            endgameEvaluation += endgamePieceTotals[pieceType][square];
            gamePhase += gamePhaseIncrement[pieceType];
        }

        // Use tapered evaluation
        int midgamePhase = gamePhase;
        if (midgamePhase > 24) {
            midgamePhase = 24;
        }
        int endgamePhase = 24 - midgamePhase;
        // Return score from the perspective of the turn player
        return (board.sideToMove() == Color::WHITE ? 1 : -1) * (midgamePhase * midgameEvaluation + endgamePhase * endgameEvaluation)/24;
    }

    inline int evaluate(Board &board) {
        return nnue.Evaluate(board.sideToMove());
        // return pstEval(board);
    }
};

// todo: incrementally update eval