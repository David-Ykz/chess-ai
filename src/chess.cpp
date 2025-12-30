#include "../include/nnue.h"
using namespace chess;

// Explicit template instantiation
template void Board::makeMove<false>(Move, NNUE::Net&);
template void Board::makeMove<true>(Move, NNUE::Net&);
template void Board::unmakeMove<true>(Move, NNUE::Net&);
template void Board::unmakeMove<false>(Move, NNUE::Net&);
template void Board::placePiece<false>(Piece piece, Square, Square kSQ_White, Square kSQ_Black, NNUE::Net&);
template void Board::placePiece<true>(Piece piece, Square sq, Square kSQ_White, Square kSQ_Black, NNUE::Net&);
template void Board::removePiece<false>(Piece piece, Square sq, Square kSQ_White, Square kSQ_Black, NNUE::Net&);
template void Board::removePiece<true>(Piece piece, Square sq, Square kSQ_White, Square kSQ_Black, NNUE::Net&);
template void Board::movePiece<false>(Piece piece, Square fromSq, Square toSq, Square kSQ_White, Square kSQ_Black, NNUE::Net&);
template void Board::movePiece<true>(Piece piece, Square fromSq, Square toSq, Square kSQ_White, Square kSQ_Black, NNUE::Net&);


Board::Board(NNUE::Net &nnue, std::string_view fen, bool chess960) {        
    prev_states_.reserve(256);
    chess960_ = chess960;
    assert(setFenInternal<true>(constants::STARTPOS));
    setFenInternal<true>(fen);
    nnue.reset_accumulators();
    refresh(nnue);
}




void Board::refresh(NNUE::Net &nnue) { nnue.refresh(*this); }

template <bool EXACT, bool updateNNUE>
void Board::makeMove(const Move move, NNUE::Net &nnue) {
    const auto capture  = at(move.to()) != Piece::NONE && move.typeOf() != Move::CASTLING;
    const auto captured = at(move.to());
    const auto pt       = at<PieceType>(move.from());

    // Validate side to move
    assert((at(move.from()) < Piece::BLACKPAWN) == (stm_ == Color::WHITE));

    prev_states_.emplace_back(key_, cr_, ep_sq_, hfm_, captured);

    if constexpr (updateNNUE) nnue.push();

    hfm_++;
    plies_++;

    const Square kSQ_White = kingSq(Color::WHITE);
    const Square kSQ_Black = kingSq(Color::BLACK);

    if (ep_sq_ != Square::NO_SQ) key_ ^= Zobrist::enpassant(ep_sq_.file());
    ep_sq_ = Square::NO_SQ;

    if (capture) {
        // removePiece(captured, move.to());
        removePiece<updateNNUE>(captured, move.to(), kSQ_White, kSQ_Black, nnue);

        hfm_ = 0;
        key_ ^= Zobrist::piece(captured, move.to());

        // remove castling rights if rook is captured
        if (captured.type() == PieceType::ROOK && Rank::back_rank(move.to().rank(), ~stm_)) {
            const auto king_sq = kingSq(~stm_);
            const auto file    = CastlingRights::closestSide(move.to(), king_sq);

            if (cr_.getRookFile(~stm_, file) == move.to().file()) {
                key_ ^= Zobrist::castlingIndex(cr_.clear(~stm_, file));
            }
        }
    }

    // remove castling rights if king moves
    if (pt == PieceType::KING && cr_.has(stm_)) {
        key_ ^= Zobrist::castling(cr_.hashIndex());
        cr_.clear(stm_);
        key_ ^= Zobrist::castling(cr_.hashIndex());
    } else if (pt == PieceType::ROOK && Square::back_rank(move.from(), stm_)) {
        const auto king_sq = kingSq(stm_);
        const auto file    = CastlingRights::closestSide(move.from(), king_sq);

        // remove castling rights if rook moves from back rank
        if (cr_.getRookFile(stm_, file) == move.from().file()) {
            key_ ^= Zobrist::castlingIndex(cr_.clear(stm_, file));
        }
    } else if (pt == PieceType::PAWN) {
        hfm_ = 0;

        // double push
        if (Square::value_distance(move.to(), move.from()) == 16) {
            // imaginary attacks from the ep square from the pawn which moved
            Bitboard ep_mask = attacks::pawn(stm_, move.to().ep_square());

            // add enpassant hash if enemy pawns are attacking the square
            if (static_cast<bool>(ep_mask & pieces(PieceType::PAWN, ~stm_))) {
                int found = -1;

                // check if the enemy can legally capture the pawn on the next move
                if constexpr (EXACT) {
                    const auto piece = at(move.from());

                    found = 0;

                    removePieceInternal(piece, move.from());
                    placePieceInternal(piece, move.to());

                    stm_ = ~stm_;

                    bool valid;

                    if (stm_ == Color::WHITE) {
                        valid = movegen::isEpSquareValid<Color::WHITE>(*this, move.to().ep_square());
                    } else {
                        valid = movegen::isEpSquareValid<Color::BLACK>(*this, move.to().ep_square());
                    }

                    if (valid) found = 1;

                    // undo
                    stm_ = ~stm_;

                    removePieceInternal(piece, move.to());
                    placePieceInternal(piece, move.from());
                }

                if (found != 0) {
                    assert(at(move.to().ep_square()) == Piece::NONE);
                    ep_sq_ = move.to().ep_square();
                    key_ ^= Zobrist::enpassant(move.to().ep_square().file());
                }
            }
        }
    }

    if (move.typeOf() == Move::CASTLING) {
        assert(at<PieceType>(move.from()) == PieceType::KING);
        assert(at<PieceType>(move.to()) == PieceType::ROOK);

        const bool king_side = move.to() > move.from();
        const auto rookTo    = Square::castling_rook_square(king_side, stm_);
        const auto kingTo    = Square::castling_king_square(king_side, stm_);

        const auto king = at(move.from());
        const auto rook = at(move.to());

        if (updateNNUE && (NNUE::KING_BUCKET[move.from().index() ^ (static_cast<bool>(sideToMove()) * 56)] != NNUE::KING_BUCKET[kingTo.index() ^ (static_cast<bool>(sideToMove()) * 56)] || move.from().file() + kingTo.file() == 7)) {
            removePiece(king, move.from());
            removePiece(rook, move.to());

            assert(king == Piece(PieceType::KING, stm_));
            assert(rook == Piece(PieceType::ROOK, stm_));

            placePiece(king, kingTo);
            placePiece(rook, rookTo);

            refresh(nnue);
        } else {
            removePiece<updateNNUE>(king, move.from(), kSQ_White, kSQ_Black, nnue);
            removePiece<updateNNUE>(rook, move.to(), kSQ_White, kSQ_Black, nnue);

            placePiece<updateNNUE>(king, kingTo, kSQ_White, kSQ_Black, nnue);
            placePiece<updateNNUE>(rook, rookTo, kSQ_White, kSQ_Black, nnue);
        }


        key_ ^= Zobrist::piece(king, move.from()) ^ Zobrist::piece(king, kingTo);
        key_ ^= Zobrist::piece(rook, move.to()) ^ Zobrist::piece(rook, rookTo);
    } else if (move.typeOf() == Move::PROMOTION) {
        const auto piece_pawn = Piece(PieceType::PAWN, stm_);
        const auto piece_prom = Piece(move.promotionType(), stm_);

        // removePiece(piece_pawn, move.from());
        // placePiece(piece_prom, move.to());

        removePiece<updateNNUE>(piece_pawn, move.from(), kSQ_White, kSQ_Black, nnue);
        placePiece<updateNNUE>(piece_prom, move.to(), kSQ_White, kSQ_Black, nnue);

        key_ ^= Zobrist::piece(piece_pawn, move.from()) ^ Zobrist::piece(piece_prom, move.to());
    } else {
        assert(at(move.from()) != Piece::NONE);
        assert(at(move.to()) == Piece::NONE);

        const auto piece = at(move.from());

        // removePiece(piece, move.from());
        // placePiece(piece, move.to());

        movePiece<updateNNUE>(piece, move.from(), move.to(), kSQ_White, kSQ_Black, nnue);


        key_ ^= Zobrist::piece(piece, move.from()) ^ Zobrist::piece(piece, move.to());
    }

    if (move.typeOf() == Move::ENPASSANT) {
        assert(at<PieceType>(move.to().ep_square()) == PieceType::PAWN);

        const auto piece = Piece(PieceType::PAWN, ~stm_);

        // removePiece(piece, move.to().ep_square());
        removePiece<updateNNUE>(piece, Square(move.to() ^ 8), kSQ_White, kSQ_Black, nnue);

        key_ ^= Zobrist::piece(piece, move.to().ep_square());
    }

    key_ ^= Zobrist::sideToMove();
    stm_ = ~stm_;
}


template <bool updateNNUE>
void Board::unmakeMove(const Move move, NNUE::Net &nnue) {
    const auto& prev = prev_states_.back();

    if constexpr (updateNNUE) {
        nnue.pull();
    }

    ep_sq_ = prev.enpassant;
    cr_    = prev.castling;
    hfm_   = prev.half_moves;
    stm_   = ~stm_;
    plies_--;

    if (move.typeOf() == Move::CASTLING) {
        const bool king_side    = move.to() > move.from();
        const auto rook_from_sq = Square(king_side ? File::FILE_F : File::FILE_D, move.from().rank());
        const auto king_to_sq   = Square(king_side ? File::FILE_G : File::FILE_C, move.from().rank());

        assert(at<PieceType>(rook_from_sq) == PieceType::ROOK);
        assert(at<PieceType>(king_to_sq) == PieceType::KING);

        const auto rook = at(rook_from_sq);
        const auto king = at(king_to_sq);

        removePiece(rook, rook_from_sq);
        removePiece(king, king_to_sq);

        assert(king == Piece(PieceType::KING, stm_));
        assert(rook == Piece(PieceType::ROOK, stm_));

        placePiece(king, move.from());
        placePiece(rook, move.to());

    } else if (move.typeOf() == Move::PROMOTION) {
        const auto pawn  = Piece(PieceType::PAWN, stm_);
        const auto piece = at(move.to());

        assert(piece.type() == move.promotionType());
        assert(piece.type() != PieceType::PAWN);
        assert(piece.type() != PieceType::KING);
        assert(piece.type() != PieceType::NONE);

        removePiece(piece, move.to());
        placePiece(pawn, move.from());

        if (prev.captured_piece != Piece::NONE) {
            assert(at(move.to()) == Piece::NONE);
            placePiece(prev.captured_piece, move.to());
        }

    } else {
        assert(at(move.to()) != Piece::NONE);
        assert(at(move.from()) == Piece::NONE);

        const auto piece = at(move.to());

        removePiece(piece, move.to());
        placePiece(piece, move.from());

        if (move.typeOf() == Move::ENPASSANT) {
            const auto pawn   = Piece(PieceType::PAWN, ~stm_);
            const auto pawnTo = static_cast<Square>(ep_sq_ ^ 8);

            assert(at(pawnTo) == Piece::NONE);

            placePiece(pawn, pawnTo);
        } else if (prev.captured_piece != Piece::NONE) {
            assert(at(move.to()) == Piece::NONE);

            placePiece(prev.captured_piece, move.to());
        }
    }

    key_ = prev.hash;
    prev_states_.pop_back();
}

template <bool updateNNUE> void Board::placePiece(Piece piece, Square sq, Square kSQ_White, Square kSQ_Black, NNUE::Net &nnue) {
    placePieceInternal(piece, sq);
    if constexpr (updateNNUE) nnue.template updateAccumulator<true>(piece.type(), piece.color(), sq, kSQ_White, kSQ_Black);
}
template <bool updateNNUE> void Board::removePiece(Piece piece, Square sq, Square kSQ_White, Square kSQ_Black, NNUE::Net &nnue) {
    removePieceInternal(piece, sq);
    if constexpr (updateNNUE) nnue.template updateAccumulator<false>(piece.type(), piece.color(), sq, kSQ_White, kSQ_Black);
}

template <bool updateNNUE>
void Board::movePiece(Piece piece, Square fromSq, Square toSq, Square kSQ_White, Square kSQ_Black, NNUE::Net& nnue) {
    removePieceInternal(piece, fromSq);
    placePieceInternal(piece, toSq);

    if constexpr (updateNNUE) {
        if (piece.type() == PieceType::KING && (NNUE::KING_BUCKET[fromSq.index() ^ (static_cast<bool>(sideToMove()) * 56)] != NNUE::KING_BUCKET[toSq.index() ^ (static_cast<bool>(sideToMove()) * 56)]||fromSq.file() + toSq.file() == 7)) {
            refresh(nnue);
        } else {
            nnue.updateAccumulator(piece.type(), piece.color(), fromSq, toSq, kSQ_White, kSQ_Black);
        }
    }

}
