import gymnasium as gym
import skpacman_gym
from skpacman_gym.wrappers import SimpleQLearningObservation

from brain import QLearningTable

env = gym.make('shopkeeper/skpacman-v0', render_mode="human", simple=True)
# env = gym.make('shopkeeper/skpacman-v0', simple=True)
env = SimpleQLearningObservation(env)
print(env.observation_space)

try:

    def eval(rl: QLearningTable):
        step = 0
        action = 0
        last_chosen = 0
        observation, info = env.reset()

        while True:
            if observation[0] == 1 and step - last_chosen > 3:
                # 即将开始下一次选择

                action = rl.choose_action(str(observation[1:]))
                print(action)

                last_chosen = step

            observation, reward, terminated, truncated, info = env.step(action)

            step += 1
            if terminated or truncated:
                # 内部调用update函数以接受关闭窗口指令
                env.step(0)


    rl_ = QLearningTable(actions=[0, 1, 2, 3],e_greedy=1.0)
    rl_.load("q_table_33.csv")
    eval(rl_)

finally:
    env.close()
