import gymnasium as gym
from gymnasium.wrappers import FlattenObservation
import skpacman_gym
import numpy as np

from double_DQN import DoubleDQN

env = gym.make('shopkeeper/skpacman-v0', render_mode="human")
env = FlattenObservation(env)

MEMORY_SIZE = 5000
observation, info = env.reset()

double_DQN = DoubleDQN(n_actions=5, n_features=np.prod(env.observation_space.shape).astype(np.int32),
                       memory_size=MEMORY_SIZE,
                       e_greedy_increment=0.001,
                       double_q=True)


def train(RL: DoubleDQN):
    total_steps = 0
    for episode in range(100):
        print("episode {}".format(episode))
        observation, info = env.reset()
        while True:
            action = RL.choose_action(observation)
            observation_, reward, terminated, truncated, info = env.step(action)

            RL.store_transition(observation, action, reward, observation_)

            if total_steps > 200 and (total_steps % 10 == 0):
                RL.learn()

            observation = observation_
            total_steps += 1
            if terminated or truncated:
                break

    env.close()
    RL.save()
    return RL.q


double_DQN.load()
q_double = train(double_DQN)

double_DQN.save()
double_DQN.plot_cost()
# env.close()

