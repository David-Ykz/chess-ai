import subprocess

def getEngineMoves(fen):
    engine = subprocess.Popen(["../engine/engine"], stdin=subprocess.PIPE, stdout=subprocess.PIPE, text=True)
    engine.stdin.write(fen)
    engine.stdin.write("\n")
    engine.stdin.flush()
    return engine.stdout.readline()


fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -"  # starting position
print(getEngineMoves(fen))
