# Pokémon Reinforcement Learning Engine

A high-performance, custom-built Pokémon battle simulator written in C++ and wrapped as a Gymnasium environment in Python. This project trains an autonomous Deep Q-Network (DQN) agent to master Pokémon battles, learning advanced strategies like type-matchups, PP management, and tactical switching.

## Features

* Ultra-Fast C++ Engine: The entire battle logic (damage calculations, status effects, stat stages, recoil) is written in C++ for maximum throughput, allowing for hundreds of thousands of simulated episodes in minutes.

* Seamless Python Binding: Uses pybind11 to expose the C++ environment to Python without I/O bottlenecks.

* Deep Reinforcement Learning: Integrated with Stable-Baselines3 and Gymnasium. The agent learns from scratch using a DQN algorithm.

* Dense State Representation: A custom 45-dimensional observation space encodes HP ratios, stat stages, status conditions, move power/accuracy, and STAB/effectiveness heuristics.

* Human vs. AI Mode: Play directly against your trained neural network in the terminal.

## Project Structure

`test.cpp` / `main.cpp`: The core C++ battle engine and game logic.

`bindings.cpp`: Pybind11 wrappers to expose C++ classes to Python.

`train.py`: The DQN training loop using Stable-Baselines3.

`play_vs_ai.py`: Scripts to battle against your trained RL model.

`evaluate`.py: Benchmarks the trained model's win rate against baseline heuristics.

`pokedex.csv`: Datasets containing base stats, typing, and move properties.

## Installation & Setup

### 1. Install Dependencies

Ensure you have Python 3 installed, then install the required Python libraries inside your virtual environment:
```bash
pip install gymnasium stable-baselines3 numpy pybind11
```

### 2. Compile the C++ Engine

Compile the C++ code into a shared object (.so) file that Python can import. Run this in your project root:
```bash
c++ -O3 -shared -std=c++17 -fPIC $(python3 -m pybind11 --includes) bindings.cpp -o poke_env_cpp$(python3-config --extension-suffix)
```

_(Note: Ensure your virtual environment is active so the compiler finds the correct pybind11 headers)._

## Usage

### Training the Agent

To start training the DQN model (defaults to 50,000 timesteps):
```bash
python3 train.py
```

This will generate and save a `pokemon_dqn_model.zip` file upon completion.

### Playing Against the AI

Once the model is trained, you can challenge it yourself!
```bash
python3 play_vs_ai.py
```

### Benchmarking

To test how well your agent has learned, run 10,000 automated battles against a random-action bot:
```bash
python3 evaluate.py
```

## RL Environment Design

### Action Space: `Discrete(10)`

* `0-3`: Use Move 1, 2, 3, or 4.

* `4-9`: Switch to Party Member 1, 2, 3, 4, 5, or 6.

### Observation Space: `Box(45,)`

The 45-dimensional float vector includes:

* Active Pokémon (Agent & Opponent): HP ratio, Attack/Def/SpA/SpD/Speed stages (normalized), and one-hot encoded Status Conditions (Burn, Sleep, Paralyze, etc.).

* Moveset Context: For each of the 4 moves, the engine provides normalized Power, Accuracy, Type Effectiveness multiplier against the current opponent, and STAB bonuses.

* Party Health: HP ratios of the 5 benched Pokémon to inform switching logic.

### Reward Shaping

* `+ / -`  HP differentials (Damage dealt vs. Damage taken).

* `-10` Penalty for invalid actions (trying to use a move with 0 PP, switching to a fainted Pokémon).

* `+50 / -50` Knockout rewards/penalties.

* `+1000 / -1000` Match Win/Loss.

### License

This project is open-source and available under the MIT License. Pokémon and Pokémon character names are trademarks of Nintendo.