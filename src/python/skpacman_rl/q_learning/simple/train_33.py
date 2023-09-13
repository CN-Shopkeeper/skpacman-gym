import gymnasium as gym
import random

import numpy as np
from skpacman_gym.wrappers import SimpleQLearningObservation

from brain import QLearningTable

# env = gym.make('shopkeeper/skpacman-v0', render_mode="human", simple=True)
env = gym.make('shopkeeper/skpacman-v0', simple=True)
env = SimpleQLearningObservation(env)
print(env.observation_space)

try:

    def train(rl: QLearningTable):
        dir_to_coor = [[1, 0], [0, 1], [-1, 0], [0, -1]]
        for episode in range(1000):
            print("episode {}".format(episode))
            step = 0
            action = 0
            last_chosen = 0
            _reward = 0
            observation, info = env.reset(seed=episode)

            observation_ = observation
            action = rl.choose_action(str(observation[1:]))
            while True:
                if observation_[0] == 1 and step - last_chosen > 3:
                    # 即将开始下一次选择

                    # 先学习上次的

                    # 如果observation[1]>0，此时能量豆状态不会被浪费
                    if observation[1] > 0:
                        # 能量豆增益
                        bean_dir = observation[1] - 1
                        if action == bean_dir:
                            _reward += 10

                    # 如果observation_[1]<0，此时如果前往就近的能量豆状态会浪费
                    if observation[1] < 0:
                        # 能量豆减益
                        bean_dir = -observation[1] - 1
                        if action == bean_dir:
                            _reward -= 20

                    # 撞墙减益
                    # 如果前进方向是墙
                    if observation[2 + action] == 3:
                        _reward -= 100

                    rl.learn(str(observation[1:]), action, _reward, str(observation_[1:]), False)
                    observation = observation_

                    # 开始新的action选择

                    count_of_beans = sum(1 for item in observation[2:-4] if item in [1, 2])
                    # if count_of_beans == 0 and random.random() < 0.5:
                    if count_of_beans == 0:
                        # 视野内的豆子数量为0，则请求神的帮助.但是神看了它一眼，有50%的几率不予理睬
                        deus_ex_machina_arr = np.array(observation[-4:])
                        deus_ex_machina = np.random.choice(np.where(deus_ex_machina_arr == 1)[0])
                        action = rl.choose_action(str(observation[1:]), deus_ex_machina)
                    else:
                        action = rl.choose_action(str(observation[1:]))
                    last_chosen = step
                    _reward = 0

                observation_, reward, terminated, truncated, info = env.step(action)

                # 累计返回结果
                _reward += reward

                step += 1
                if terminated or truncated:
                    # 游戏结束，也需要学习一下
                    rl.learn(str(observation[1:]), action, _reward, str(observation_[1:]), terminated)

                    rl.save("q_table_33.csv")
                    break


    rl_ = QLearningTable(actions=[0, 1, 2, 3])
    rl_.load("q_table_33.csv")
    train(rl_)

finally:
    env.close()
