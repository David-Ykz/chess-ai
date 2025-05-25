using namespace chess;
using namespace std;

#include "chess-library/include/chess.hpp"
#include "pst.h"

class Evaluator {
private:
    int gamePhaseIncrement[14] = {0, 1, 1, 2, 4, 0, 0, 0, 0, 1, 1, 2, 4, 0};
    PST tables;
    int midgamePieceValues[12] = {82, 337, 365, 477, 1025, 0, -82, -337, -365, -477, -1025, 0};
    int endgamePieceValues[12] = {94, 281, 297, 512, 936, 0, -94, -281, -297, -512, -936, 0};
    int midgamePieceTotals[12][64];
    int endgamePieceTotals[12][64];

public:
    Evaluator() {
        // Initialize midgame and endgame psts
        for (int p = 0U; p <= 11U; p++) {
            for (int i = 0; i < 64; i++) {
                midgamePieceTotals[p][i] = midgamePieceValues[p] + tables.midgamePst[p][i];
                endgamePieceTotals[p][i] = endgamePieceValues[p] + tables.endgamePst[p][i];
            }
        }
    }

    int evaluate(Board& board) {
        int midgameEvaluation = 0;
        int endgameEvaluation = 0;
        int gamePhase = 0;
        Bitboard occ = board.occ();
        // Get piece score for each piece on the board
        while (occ) {
            uint8_t square = occ.pop();
            int pieceType = board.at(square);
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

    inline int getPieceValue(const Piece& p) {
        return midgamePieceValues[p];
    }
};