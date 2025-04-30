import asyncio
from interface import getEngineMove
from tables import getOpeningMove
import chess
import chess.pgn

async def main():
    board = chess.Board()
    startingFen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -"
    numOpeningMoves = 4

    while not board.is_game_over():
        move = ""
        if numOpeningMoves >= 0:
            move = await getOpeningMove(board.fen(), 2)
        if numOpeningMoves == 0:
            print("----------")
        if move != "":
            move = chess.Move.from_uci(move)
            numOpeningMoves -= 1
        else:
            move = chess.Move.from_uci(getEngineMove(board.fen()))

        print(move)
        if move in board.legal_moves:
            board.push(move)
        else:
            print(f"Illegal move: {move}")
            print(board.fen())

    print(chess.pgn.Game.from_board(board))


import chess
import chess.engine

def playMatch(enginePath, skillLevel):
    engine1 = chess.engine.SimpleEngine.popen_uci(enginePath)
    engine1.configure({"Skill Level": skillLevel})
    board = chess.Board()
    board.push(chess.Move.from_uci("e2e4"))
    board.push(chess.Move.from_uci("e7e5"))
    board.push(chess.Move.from_uci("g1f3"))
    board.push(chess.Move.from_uci("b8c6"))
    board.push(chess.Move.from_uci("f1b5"))
    board.push(chess.Move.from_uci("a7a6"))
    while not board.is_game_over():
        move = getEngineMove(board.fen())
        move = chess.Move.from_uci(move)
#        print(f"Move {board.fullmove_number}: {board.san(move)}")
        board.push(move)
        if board.is_game_over():
            print(chess.pgn.Game.from_board(board))
            return 0
        result = engine1.play(board, chess.engine.Limit(time=0.5))
        move = result.move
#        print(f"Move {board.fullmove_number}: {board.san(move)}")
        board.push(move)

    # Output final result
    print(chess.pgn.Game.from_board(board))

    engine1.quit()




def run_match(engine1_path, engine2_path, moves=100):
    # Start both engines using subprocess
    engine1 = chess.engine.SimpleEngine.popen_uci(engine1_path)
    engine2 = chess.engine.SimpleEngine.popen_uci(engine2_path)
    engine2.configure({"Skill level: ", 10})
    # Initialize the chess board
    board = chess.Board()

    # Alternate moves between engine1 and engine2
    current_engine = engine1
    while not board.is_game_over() and board.fullmove_number <= moves:
        # Get the best move from the current engine
        result = current_engine.play(board, chess.engine.Limit(time=2.0))  # Limit each engine's move time to 2 seconds
        move = result.move
        board.push(move)

        # Print the move and board position
        print(f"Move {board.fullmove_number}: {board.san(move)}")
        print(board)

        # Switch engines
        current_engine = engine2 if current_engine == engine1 else engine1

    # Output final result
    print("Game over!")
    print("Result:", board.result())

    # Close engines
    engine1.quit()
    engine2.quit()

# Paths to the UCI engine executables (Stockfish or others)
engine1_path = "./engine.py"  # E.g., Stockfish path
engine2_path = "../stockfish/stockfish-ubuntu-x86-64-avx2"  # E.g., another engine path

enginePath = "../stockfish/stockfish-ubuntu-x86-64-avx2"

for i in range(5):
    playMatch(enginePath, 10)


# for i in range(5):
#     for j in range(10):
#         print(f"---------- Playing match: skill level = {i + 5} ----------")
#         playMatch(enginePath, i + 5)



# Run the async main
#asyncio.run(main())
