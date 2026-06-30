import gymnasium as gym
from gymnasium import spaces
import numpy as np
from stable_baselines3 import DQN
import poke_env_cpp

class PokemonEnv(gym.Env):
    def __init__(self):
        super().__init__()
        self.action_space = spaces.Discrete(10)
        self.observation_space = spaces.Box(low=-1.0, high=2.0, shape=(45,), dtype=np.float32)
        self.env = None

    def reset(self, seed=None, options=None):
        super().reset(seed=seed)
        agent_party = [poke_env_cpp.getRandomPokemon() for _ in range(6)]
        opp_party = [poke_env_cpp.getRandomPokemon() for _ in range(6)]
        self.env = poke_env_cpp.RLEnvironment(agent_party, opp_party)
        obs = self.env.reset()
        return np.array(obs, dtype=np.float32), {}
    
    def step(self, action, opp_action=-1):
        result = self.env.step(int(action), int(opp_action))
        obs = np.array(result.next_state, dtype=np.float32)
        reward = float(result.reward)
        done = bool(result.done)
        truncated = False
        info = {}
        return obs, reward, done, truncated, info

if __name__ == "__main__":
    env = PokemonEnv()
    model = DQN.load("pokemon_dqn_model")
    
    obs, info = env.reset()
    done = False
    
    print("\nBATTLE START\n")
    turn_count = 0
    
    while not done:
        turn_count+=1
        print(f"\nTurn {turn_count}")
        
        valid_input = False
        opp_action = -1

        print(f"\nYour Pokemon : ")
        env.env.displayParty()

        env.env.render()
        
        while not valid_input:
            print("What do you want to do?")
            print("F : Fight")
            print("S : Switch")
            
            choice = input().strip().lower()
            
            if choice in ['f', 'fight']:
                env.env.showMoves()
                try:
                    move_choice = int(input().strip())
                    if 1 <= move_choice <= 4:
                        opp_action = move_choice - 1
                        valid_input = True
                    else:
                        print("Invalid choice! Enter again.")
                except ValueError:
                    print("Invalid! Enter again.")

            elif choice in ['s', 'switch'] :
                env.env.showSwitches()
                try:
                    switch_choice = int(input().strip())
                    if 1<=switch_choice<=6:
                        opp_action = switch_choice + 3
                        valid_input = True
                    else :
                        print("Invalid choice! Enter again : ")
                except ValueError:
                        print("Invalid! Enter again : ")
            else :
                print("Invalid choice! Enter again : ")
                
        action, _states = model.predict(obs, deterministic=True)
        obs, reward, done, truncated, info = env.step(action, opp_action)
        
    print("BATTLE OVER!\n")