import gymnasium as gym
import skpacman_gym
from skpacman_gym.wrappers import SimpleQLearningObservation

env = gym.make('shopkeeper/skpacman-v0', render_mode="human", simple=True)
env = SimpleQLearningObservation(env, performance=True)
print(env.observation_space)

try:
    step = 0
    action = 0
    last_chosen = 0
    _reward = 0
    observation, info = env.reset()
    print(observation)
    while True:
        if observation[0] == 1 and step - last_chosen > 3:
            # 即将开始下一次选择

            # 先学习上次的
            # todo learn with _reward

            # 开始新的action选择
            action = env.action_space.sample()
            last_chosen = step
            _reward = 0

        observation, reward, terminated, truncated, info = env.step(action)
        if not env.observation_space.contains(observation):
            print(observation)

        # 累计返回结果
        _reward += reward

        step += 1
        if terminated or truncated:
            # 游戏结束，也需要学习一下
            # todo learn with _reward

            step = 0
            action = 0
            last_chosen = 0
            _reward = 0
            observation, info = env.reset()


finally:
    env.close()
