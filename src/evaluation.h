#pragma once

#include "nn/probe.h"
#include "pst.h"

class Evaluation {
private:
    int midgamePieceValues[14] = {82, 337, 365, 477, 1025, 0, 0, 0, -82, -337, -365, -477, -1025, 0};
    int endgamePieceValues[14] = {94, 281, 297, 512, 936, 0, 0, 0, -94, -281, -297, -512, -936, 0};
    int gamePhaseIncrement[14] = {0, 1, 1, 2, 4, 0, 0, 0, 0, 1, 1, 2, 4, 0};
    PST tables;

public:
    void initialize(string modelName) {
        Stockfish::Probe::init(modelName.c_str(), modelName.c_str());
    }

    template <Color Us>
    inline int evaluate(Position& position) {
        int midgameEvaluation = 0;
        int endgameEvaluation = 0;
        int gamePhase = 0;
        for (int i = WHITE_PAWN; i < NO_PIECE; ++i) {
            Piece piece = static_cast<Piece>(i);
            Bitboard bitboard = position.bitboard_of(piece);
            while (bitboard) {
                int square = __builtin_ctzll(bitboard);
                bitboard &= bitboard - 1;
                midgameEvaluation += midgamePieceValues[i] + tables.midgamePst[i][square];
                endgameEvaluation += endgamePieceValues[i] + tables.endgamePst[i][square];
                gamePhase += gamePhaseIncrement[i];
            }
        }
        int midgamePhase = gamePhase;
        if (midgamePhase < 24) {
            midgamePhase = 24;
        }
        int endgamePhase = 24 - midgamePhase;
        if constexpr (Us == BLACK) {
            return -(midgamePhase * midgameEvaluation + endgamePhase * endgameEvaluation)/24;
        }

        return (midgamePhase * midgameEvaluation + endgamePhase * endgameEvaluation)/24;
    }


    template <Color Us>
    static int nnueevaluate(Position& position) {
        int pieces[32];
        int squares[32];
        int index = 0;
        for (int i = WHITE_PAWN; i < NO_PIECE; ++i) {
            Bitboard bitboard = position.bitboard_of(static_cast<Piece>(i));
            while (bitboard) {
                int square = __builtin_ctzll(bitboard);
                bitboard &= bitboard - 1;
                pieces[index] = i + 1;
                squares[index] = square;
                ++index;
            }
        }
        if constexpr (Us == WHITE) {
            return Stockfish::Probe::eval(pieces, squares, index, true, 0);
        } else {
            return Stockfish::Probe::eval(pieces, squares, index, false, 0);
        }
    }
};