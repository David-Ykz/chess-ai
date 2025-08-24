# Chess AI

## Overview
This project is a custom-built chess engine in C++ designed to play at a high level (2000+ elo). It combines classical search algorithms and techniques with modern machine learning models to evaluate positions and make strong moves. It is the latest iteration in my journey to create a 3000 elo engine, and a significant upgrade over my previous version (rated 1800 elo), which can be found [here](https://github.com/David-Ykz/Java-Chess-AI).

---

## Techniques Used
The engine uses several well-established techniques in computer chess, taking inspiration from engines like Stockfish. Notable techniques include:

- Fast bitboard-based move generation using [Disservin's chess library](https://github.com/Disservin/chess-library)
- Alpha-Beta pruning
- Quiescence search
- Move ordering
- Killer moves
- Transposition tables
- Null move pruning
- Late move reductions
- Reverse futility pruning
- Iterative deepening
- Aspiration windows
- Principle variation search

---

## Neural Network
This project integrates a neural network based off of Stockfish's [NNUE](https://www.chessprogramming.org/Stockfish_NNUE) using an [NNUE prober](https://github.com/VedantJoshi1409/stockfish_nnue_probe). Due to constraints, the prober does not implement an accumulator, thus we are unable to leverage the "efficiently updatable" aspect of the NNUE. 
A sample small-scale model was trained using Pytorch following [Stockfish's NNUE training guide](https://github.com/official-stockfish/nnue-pytorch), but for testing a full scaled model was imported from Stockfish's official model repository, as I do not have a GPU suitable for training large models. 
In switching over from a piece square tables approach (PST) to a neural network, the engine became slower (measured in nodes/sec) by a factor of 200 largely due to the lack of an accumulator causing long inference times. However, the search depth remained mostly the same, likely due to better evaluations from the evaluation function leading to better pruning outcomes. 

---

## Strength
Prior to the introduction of a neural network, the engine's playing strength was around 2200 elo on Lichess. By switching from the traditional PST approach to a neural network, the engine gained around 200 elo points, with further optimizations increasing the engine's rating to 2500 elo. 
These ratings were determined by testing the engine against other chess engines around 2500 elo like Stockfish* or [Euwe](https://lichess.org/@/Euwe-chess-engine).

Some of these games have been included here:
- [bot v7 vs Euwe](https://lichess.org/rmDjEmnr)
- [bot v7 vs Stockfish 15](https://lichess.org/zJvuVmb0)
- [Euwe vs bot v7](https://lichess.org/EMcCrCaf)


*Testing was done with Stockfish level 10 - 15 (2264 - 2619 elo). More information can be found on this [thread](https://chess.stackexchange.com/questions/29860/is-there-a-list-of-approximate-elo-ratings-for-each-stockfish-level)  

---
