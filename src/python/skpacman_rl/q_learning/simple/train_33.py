import gymnasium as gym
import skpacman_gym
import random

from skpacman_gym.wrappers import SimpleObservation

from brain import QLearningTable

# env = gym.make('shopkeeper/skpacman-v0', render_mode="human", simple=True)
env = gym.make('shopkeeper/skpacman-v0', simple=True)
env = SimpleObservation(env, size=(3, 3))
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
            observation, info = env.reset()
            observation[1] = (observation[1] - 1) // (33 * 5) + 1
            observation_ = observation
            action = rl.choose_action(str(observation[1:-1]))
            while True:
                if observation_[0] == 1 and step - last_chosen > 3:
                    # 即将开始下一次选择

                    # 先学习上次的

                    # 时间减益
                    if observation_[1] == 0:
                        _reward -= 1

                    # 撞墙减益
                    pac_x = 3 // 2
                    pac_y = 3 // 2
                    dir_x, dir_y = dir_to_coor[action]
                    # 如果前进方向是墙
                    if observation_[2 + (pac_x + dir_x + (pac_y + dir_y) * 3)] == 3:
                        _reward -= 1

                    rl.learn(str(observation[1:-1]), action, _reward, str(observation_[1:-1]), False)
                    observation = observation_

                    # 开始新的action选择

                    count_of_beans = sum(1 for item in observation[2:-1] if item in [1, 2])
                    if count_of_beans == 0 and random.random() < 0.5:
                        # 视野内的豆子数量为0，则请求神的帮助.但是神看了它一眼，有50%的几率不予理睬
                        action = rl.choose_action(str(observation[1:-1]), observation[-1])
                    else:
                        action = rl.choose_action(str(observation[1:-1]))
                    last_chosen = step
                    _reward = 0

                observation_, reward, terminated, truncated, info = env.step(action)
                observation_[1] = (observation_[1] - 1) // (33 * 5) + 1
                # print(observation_)

                # 累计返回结果
                _reward += reward

                step += 1
                if terminated or truncated:
                    # 游戏结束，也需要学习一下
                    rl.learn(str(observation[1:-1]), action, _reward, str(observation_[1:-1]), terminated)

                    rl.save("q_table_33.csv")
                    break


    rl_ = QLearningTable(actions=[0, 1, 2, 3])
    rl_.load("q_table_33.csv")
    train(rl_)

finally:
    env.close()
