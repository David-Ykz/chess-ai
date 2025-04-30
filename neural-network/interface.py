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
        move = move[0:5]
        if move[4:5] == " ":
            move = move[0:4]
        else:
            move = move[0:4] + str(move[4:5]).lower()
        moveDictionary[move] = int(value)
    
    return moveDictionary

def getEngineMove(fen):
    if "w" in fen:
        engine = subprocess.Popen(["../engine/midgamepst"], stdin=subprocess.PIPE, stdout=subprocess.PIPE, text=True)
    else:
        engine = subprocess.Popen(["../engine/basicpst"], stdin=subprocess.PIPE, stdout=subprocess.PIPE, text=True)
    engine.stdin.write(fen)
    engine.stdin.write("\n")
    engine.stdin.flush()
    move = engine.stdout.readline().rstrip("\n")
    engine.stdin.close()
    engine.stdout.close()
    engine.wait()
    
    if move[4:5] == " ":
        move = move[0:4]
    else:
        move = move[0:4] + str(move[4:5]).lower()

    return move

def getEngineMoves(fen):
    if "b" in fen:
        engine = subprocess.Popen(["../engine/engine2"], stdin=subprocess.PIPE, stdout=subprocess.PIPE, text=True)
    else:
        engine = subprocess.Popen(["../engine/engine"], stdin=subprocess.PIPE, stdout=subprocess.PIPE, text=True)
    engine.stdin.write(fen)
    engine.stdin.write("\n")
    engine.stdin.flush()
    candidateMovesRaw = engine.stdout.readline()
    engine.stdin.close()
    engine.stdout.close()
    engine.wait()
    print(candidateMovesRaw)
    return parseMoves(candidateMovesRaw)

def getTopEngineMove(fen):
    moves = getEngineMoves(fen)
    return max(moves, key=moves.get)
    # if ("w" in fen):
    #     return max(moves, key=moves.get)
    # elif ("b" in fen):
    #     return min(moves, key=moves.get)
    # else:
    #     print("Bad fen")

# fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -"  # starting position
# print(getEngineMoves(fen))
