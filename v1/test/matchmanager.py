import asyncio
import chess
import chess.pgn
import chess.engine

def run_match(engine1_path, engine2_path):
    # Start both engines using subprocess
    engine1 = chess.engine.SimpleEngine.popen_uci(engine1_path)
    engine2 = chess.engine.SimpleEngine.popen_uci(engine2_path)
    engine2.configure({"Skill level": 16})
    # Initialize the chess board
    board = chess.Board()

    # Alternate moves between engine1 and engine2
    current_engine = engine1
    while not board.is_game_over():
        # Get the best move from the current engine
        if current_engine == engine1:
            result = current_engine.play(board, chess.engine.Limit(time=10))
        else:
            result = current_engine.play(board, chess.engine.Limit(time=0.5))
        move = result.move
        print(f"{"us" if current_engine == engine2 else "them"}: {move}")
        board.push(move)

        # Print the move and board position

        # Switch engines
        current_engine = engine2 if current_engine == engine1 else engine1

    # Output final result
    print("Game over!")
    print(chess.pgn.Game.from_board(board))

    # Close engines
    engine1.quit()
    engine2.quit()

# Paths to the UCI engine executables (Stockfish or others)
engine1_path = "./engines/engine"
engine2_path = "./engines/stockfish-ubuntu-x86-64-avx2"
# engine2_path = "./engines/Euwe-v2.1.0"

for i in range(10):
    asyncio.run(run_match(engine1_path, engine2_path))
print("Done")