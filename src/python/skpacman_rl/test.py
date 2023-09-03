import gymnasium as gym
import skpacman_gym

env = gym.make('shopkeeper/skpacman-v0', render_mode="human")
observation, info = env.reset()

for _ in range(1000):
    action = env.action_space.sample()  # agent policy that uses the observation and info
    observation, reward, terminated, truncated, info = env.step(action)
    if not env.observation_space.contains(observation):
        print(observation)
    if terminated or truncated:
        observation, info = env.reset()

env.close()
