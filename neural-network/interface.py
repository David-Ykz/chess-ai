import subprocess

def parseMoves(s):
    moveDictionary = {}
    pairs = s.strip().split(',')

    for pair in pairs:
        if not pair:
            continue
        move, value = pair.split('|')        
        if move == "a1a1":
            continue
        moveDictionary[move] = int(value)
    
    return moveDictionary


def getEngineMoves(fen):
    engine = subprocess.Popen(["../engine/engine"], stdin=subprocess.PIPE, stdout=subprocess.PIPE, text=True)
    engine.stdin.write(fen)
    engine.stdin.write("\n")
    engine.stdin.flush()
    candidateMovesRaw = engine.stdout.readline()
    return parseMoves(candidateMovesRaw)

fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -"  # starting position
print(getEngineMoves(fen))
