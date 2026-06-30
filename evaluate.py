import numpy as np
from stable_baselines3 import DQN
import poke_env_cpp

def play_battle(model, verbose=False):
    agent_party = [poke_env_cpp.getRandomPokemon() for _ in range(6)]
    opp_party = [poke_env_cpp.getRandomPokemon() for _ in range(6)]
    
    env = poke_env_cpp.RLEnvironment(agent_party, opp_party)
    state = env.reset()
    done = False
    turn_count = 1
    
    if verbose:
        print("\n=== BATTLE START: RL AGENT vs RANDOM BOT ===")
    
    while not done:
        if verbose:
            print(f"\n--- Turn {turn_count} ---")
            env.render()
        
        obs = np.array(state, dtype=np.float32)
        action, _states = model.predict(obs, deterministic=True)
        
        result = env.step(int(action))
        
        state = result.next_state
        done = result.done
        turn_count += 1
        
    if verbose:
        print("\n=== BATTLE OVER ===")
        if result.reward > 500:
            print("WINNER: RL Agent!")
        else:
            print("WINNER: Random Bot!")

    if result.reward > 500:
        return 1
    else:
        return 0

if __name__ == "__main__":
    print("Loading Trained DQN Model...")
    model = DQN.load("pokemon_dqn_model")
    
    total_iterations = 100000
    wins = 0
    
    print(f"Running {total_iterations} battles. Please wait...")
    for i in range(total_iterations):
        wins += play_battle(model, verbose=False)
        
        if (i + 1) % 100 == 0:
            print(f"Completed {i + 1} battles... Current Win Rate: {(wins / (i + 1)) * 100:.2f}%")
    
    win_rate = (wins / total_iterations) * 100
    print(f"\nFinal Win Rate over {total_iterations} games: {win_rate:.2f}%")