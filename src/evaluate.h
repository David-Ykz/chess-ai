using namespace chess;
using namespace std;

#include "chess-library/include/chess.hpp"
#include "pst.h"
#include "nn/probe.h"

class Evaluator {
private:
    int gamePhaseIncrement[14] = {0, 1, 1, 2, 4, 0, 0, 0, 0, 1, 1, 2, 4, 0};
    PST tables;
    int midgamePieceValues[12] = {82, 337, 365, 477, 1025, 0, -82, -337, -365, -477, -1025, 0};
    int endgamePieceValues[12] = {94, 281, 297, 512, 936, 0, -94, -281, -297, -512, -936, 0};
    int midgamePieceTotals[12][64];
    int endgamePieceTotals[12][64];
    string MODEL_NAME = "nn/nn-1111cefa1111.nnue";

public:
    Evaluator() {
        Stockfish::Probe::init(MODEL_NAME.c_str(), MODEL_NAME.c_str());
        // Initialize midgame and endgame psts
        for (int p = 0U; p <= 11U; p++) {
            for (int i = 0; i < 64; i++) {
                midgamePieceTotals[p][i] = midgamePieceValues[p] + tables.midgamePst[p][i];
                endgamePieceTotals[p][i] = endgamePieceValues[p] + tables.endgamePst[p][i];
            }
        }
    }

    inline int basicEvaluation(Board& board) {
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

    inline int heuristicEvaluation(Board& board) {
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

    inline int nnEvaluate(Board& board) {
        int pieces[32];
        int squares[32];
        int index;
        int gamePhase = 0;
        Bitboard occ = board.occ();
        // Get type and location of each piece
        while (occ) {
            uint8_t square = occ.pop();
            uint8_t pieceType = board.at(square);
            // Convert piece types into format compatible with probe format
            pieces[index] = pieceType + (pieceType > 5 ? 3 : 1);
            squares[index] = square;
            ++index;
        }
        return Stockfish::Probe::eval(pieces, squares, index, board.sideToMove() == Color::WHITE, board.halfMoveClock());
    }



    inline int evaluate(Board& board) {
        int midgameEvaluation = 0;
        int endgameEvaluation = 0;
        int pieces[32];
        int squares[32];
        int index;
        int gamePhase = 0;
        Bitboard occ = board.occ();
        // Get piece score for each piece on the board
        while (occ) {
            uint8_t square = occ.pop();
            uint8_t pieceType = board.at(square);
            pieces[index] = pieceType + (pieceType > 5 ? 3 : 1);
            squares[index] = square;
            // midgameEvaluation += midgamePieceTotals[pieceType][square];
            // endgameEvaluation += endgamePieceTotals[pieceType][square];
            // gamePhase += gamePhaseIncrement[pieceType];
            ++index;
        }

        // Use tapered evaluation
        // int midgamePhase = gamePhase;
        // if (midgamePhase > 24) {
        //     midgamePhase = 24;
        // }
        // int endgamePhase = 24 - midgamePhase;
        // int eval = (midgamePhase * midgameEvaluation + endgamePhase * endgameEvaluation)/24;

        // if (abs(eval) < 100) {
           return Stockfish::Probe::eval(pieces, squares, index, board.sideToMove() == Color::WHITE, board.halfMoveClock());
        // }

        // Return score from the perspective of the turn player
        // return board.sideToMove() == Color::WHITE ? eval : -eval;
    }


    // inline int evaluate(Board& board) {
    //     return heuristicEvaluation(board);
    // }

    inline uint8_t getPieceValue(const Piece& p) {
        return midgamePieceValues[p];
    }
};