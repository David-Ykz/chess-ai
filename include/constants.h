#include "chess.hpp"

constexpr int INFINITY = 32000;
constexpr int MAX_PLY = 128;

constexpr uint64_t TIME_MARGIN = 50;
const std::string NNUE_NAME = "nets/nn-c288c895ea92.nnue";
constexpr int NUM_THREADS = 4;

// Move ordering bonuses
constexpr int TT_MOVE_BONUS = 16000;
constexpr int GOOD_CAPTURE_BONUS = 10000;
constexpr int COUNTER_BONUS = 8500;
constexpr int KILLER_BONUS[2] = {8000, 4000};
constexpr int MAX_HISTORY_BONUS = 3000;

// MVV LVA scores and piece values taken from Rice
constexpr int mvv_lva[12][12] = {
    105, 205, 305, 405, 505, 605, 105, 205, 305, 405, 505, 605, 104, 204, 304,
    404, 504, 604, 104, 204, 304, 404, 504, 604, 103, 203, 303, 403, 503, 603,
    103, 203, 303, 403, 503, 603, 102, 202, 302, 402, 502, 602, 102, 202, 302,
    402, 502, 602, 101, 201, 301, 401, 501, 601, 101, 201, 301, 401, 501, 601,
    100, 200, 300, 400, 500, 600, 100, 200, 300, 400, 500, 600,

    105, 205, 305, 405, 505, 605, 105, 205, 305, 405, 505, 605, 104, 204, 304,
    404, 504, 604, 104, 204, 304, 404, 504, 604, 103, 203, 303, 403, 503, 603,
    103, 203, 303, 403, 503, 603, 102, 202, 302, 402, 502, 602, 102, 202, 302,
    402, 502, 602, 101, 201, 301, 401, 501, 601, 101, 201, 301, 401, 501, 601,
    100, 200, 300, 400, 500, 600, 100, 200, 300, 400, 500, 600
};
constexpr int pieceValues[7] = { 93, 308, 346, 521, 994, 20000, 0};
constexpr PieceType types[] = {PieceType::PAWN, PieceType::KNIGHT, PieceType::BISHOP, PieceType::ROOK, PieceType::QUEEN, PieceType::KING};
