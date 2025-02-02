# Chess Game Simulation with Parallel MinMax Alpha-Beta

## Overview
This codebase simulates a game of chess between two computer players using a MinMax Alpha-Beta algorithm. The primary objective is to enhance the game’s performance by parallelizing the MinMax algorithm, using OpenMP to execute different depths of the search tree in parallel. The project also includes tools for simulating games, processing results, and analyzing the performance of sequential and parallelized versions.

## Codebase:
- `jeu.c / jeu.h`: Contains the logic for the chess game, including the MinMax Alpha-Beta pruning algorithm. A parallelized version of the MinMax Alpha-Beta function using OpenMP is added to improve performance.
- `simulation.c`: Simulates a full game between two AI players using the MinMax algorithm.
- `scripts`: A collection of shell scripts that automate the running of simulations, profiling, and other tasks related to the project.
- `logs`: Stores the stdout logs of the simulated games, which are later processed for performance analysis and game statistics.
- `process.ipynb`: A Jupyter Notebook used for analyzing the game simulation results, including parallelization metrics and execution times.

## How to Run
1. **Run interactively**: Use the script `scripts/run.sh` choose either playing against the computer, or watch them playing. An emoji based view has been added for clarity.
1. **Run a Simulation**: Use the script `scripts/simulate.sh` to run a full simulation of the game.

## Acknowledgements
The codebase is based on the work of **Professor Hidouci Walid Khaled** within the [*Complexity and Problem Solving*](https://sites.google.com/site/hidouciwk/cours/1-complexity-and-problems-solving?authuser=0) module at [*Ecole Nationale Superieure d'Informatique*](https://esi.dz/). For more details on the course content and game code, visit our professor's official website [here](https://sites.google.com/site/hidouciwk/home?authuser=0).

## Contributors
- [Kamel BROUTHEN](https://github.com/BrouthenKamel)
- [Abdelaziz AKEB](https://github.com/4zz0u4k)
