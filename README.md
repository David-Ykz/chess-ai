## Overview
This project is the latest iteration in my journey of building chess engines, with a previous [C++ version](https://github.com/David-Ykz/chess-ai/tree/main/v1) (rated 2200 elo) and [Java version](https://github.com/David-Ykz/Java-Chess-AI) (rated 1800 elo). It combines Alpha-Beta search with an Efficiently Updatable Neural Network (NNUE) to search and evaluate positions to find optimal moves, and also supports basic UCI commands. 

## Techniques Used
### Move generation:
- Bitboard-based move generation
### Move ordering:
- Most Valuable Victim - Least Valuable Attacker (MVV-LVA)
- Killer moves
- History heuristic
- Countermove heuristic
- Static exchange evaluation (SEE)
### Search pruning:
- Alpha-Beta pruning
- Null move pruning
- Late move reductions
- Futility pruning
- Reverse futility pruning
- SEE pruning
### Other:
- Quiescence search
- Transposition tables
- Iterative deepening
- Aspiration windows
- Multithreading (lazy SMP)

## Neural Network

This project implements an Efficiently Updatable Neural Network (NNUE) to evaluate chess positions. The project uses a HalfK architecture with 16 king buckets: `(12 * 64 * 16 -> 512) x 2 -> 1`, and integrates an accumulator to cache evaluations, allowing the engine to incrementally update the hidden layer instead of recomputing the entire matrix. 

## Rating Evaluation

The engine is rated at 3308 elo (blitz). This rating was calculated using [cutechess](https://cutechess.com/) to run thousands of matches between other engines. Cutechess was also used to validate changes and run regression tests, along with helping fine-tune parameters for heuristics like Late Move Reductions. 


## Acknowledgements

Special thanks to
- [Disservin](https://github.com/Disservin) for providing the move generation library
- [rafid-dev](https://github.com/rafid-dev) for providing the NNUE model and SEE weights
- [Nonlinear2](https://github.com/Nonlinear2), whose engine (Bread 2.0.0) was used for final rating evaluations
