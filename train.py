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
        self.agent_party = None
        self.opp_party = None

    def reset(self, seed=None, options=None):
        super().reset(seed=seed)

        self.agent_party = [poke_env_cpp.getRandomPokemon() for _ in range(6)]
        self.opp_party = [poke_env_cpp.getRandomPokemon() for _ in range(6)]
        
        self.env = poke_env_cpp.RLEnvironment(self.agent_party, self.opp_party)
        obs = self.env.reset()

        return np.array(obs, dtype=np.float32), {}
    
    def step(self, action):
        result = self.env.step(int(action))

        obs = np.array(result.next_state, dtype=np.float32)
        reward = float(result.reward)
        done = bool(result.done)
        truncated = False
        info = {}

        return obs, reward, done, truncated, info
    
if __name__ == "__main__":
    env = PokemonEnv()

    model = DQN("MlpPolicy", env, verbose=1, learning_rate=1e-3, buffer_size=100000)

    model.learn(total_timesteps=100000)

    model.save("pokemon_dqn_model")