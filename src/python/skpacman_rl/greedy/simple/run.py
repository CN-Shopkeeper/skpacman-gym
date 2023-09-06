import gymnasium as gym
import skpacman_gym
import numpy as np
from skpacman_gym.wrappers import SKFlattenObservation, GreedyObservation

env = gym.make('shopkeeper/skpacman-v0', render_mode="human", simple=True)
env = GreedyObservation(env)

try:
    step = 0
    action = 0
    last_chosen = 0
    _reward = 0
    observation, info = env.reset()
    zero_action = -1
    while True:
        if observation[0] == 1 and step - last_chosen > 3:
            # 开始新的action选择
            # 找到最大值
            action_weights = observation[1:]
            max_value = np.max(action_weights)
            # 找到所有最大值的索引
            max_indices = np.where(action_weights == max_value)[0]
            # if max_value <= 1:
            #     # 如果最大元素为1，说明可能附近找不到豆子。则一直沿着一个方向前进，当碰墙时就会自动选择别的
            #     action = np.argmax(action_weights)
            #     if zero_action == -1 or action_weights[zero_action] == 0:
            #         zero_action = np.random.choice(max_indices)
            #     action = zero_action
            # else:
                # 随机选择一个最大值的索引
            action = np.random.choice(max_indices)

            # action = env.action_space.sample()
            last_chosen = step
            _reward = 0

        observation, reward, terminated, truncated, info = env.step(action)
        if not env.observation_space.contains(observation):
            print(observation)

        # 累计返回结果
        _reward += reward

        step += 1
        if terminated or truncated:
            # 内部调用update函数以接受关闭窗口指令
            env.step(0)


finally:
    env.close()
