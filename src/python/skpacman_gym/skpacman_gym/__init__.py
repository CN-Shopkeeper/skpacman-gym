import gymnasium
from gymnasium.envs.registration import register

register(
    id="shopkeeper/skpacman-v0",
    entry_point="skpacman_gym.envs:SKPacmanEnv",
    # max_episode_steps=30000,
)