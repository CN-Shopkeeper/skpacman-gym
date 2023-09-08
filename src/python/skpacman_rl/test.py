import gymnasium as gym
import skpacman_gym
from gymnasium.wrappers import FlattenObservation
from skpacman_gym.wrappers import SKFlattenObservation, SimpleGreedyObservation, SimpleObservation

env = gym.make('shopkeeper/skpacman-v0', render_mode="human", simple=True)

try:
    env = SimpleObservation(env)
    print(env.observation_space)
    observation, info = env.reset()
    print(observation)


finally:
    env.close()
