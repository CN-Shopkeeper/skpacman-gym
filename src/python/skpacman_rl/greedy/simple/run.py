import gymnasium as gym
import skpacman_gym
import numpy as np
from skpacman_gym.wrappers import SKFlattenObservation, SimpleGreedyObservation

env = gym.make('shopkeeper/skpacman-v0', render_mode="human", simple=True)
env = SimpleGreedyObservation(env)

try:
    step = 0
    action = 0
    last_chosen = 0
    observation, info = env.reset()
    while True:
        if observation[0] == 1 and step - last_chosen > 3:
            # 开始新的action选择
            # 找到最大值
            action_weights = observation[1:]
            max_value = np.max(action_weights)
            # 找到所有最大值的索引
            max_indices = np.where(action_weights == max_value)[0]
            action = np.random.choice(max_indices)

            # action = env.action_space.sample()
            last_chosen = step

        observation, reward, terminated, truncated, info = env.step(action)
        if not env.observation_space.contains(observation):
            print(observation)

        step += 1
        if terminated or truncated:
            # 内部调用update函数以接受关闭窗口指令
            env.step(0)


finally:
    env.close()
