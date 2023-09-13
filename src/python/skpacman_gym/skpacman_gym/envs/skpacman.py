import gymnasium as gym
from gymnasium import spaces

import numpy as np

from . import binder


class SKPacmanEnv(gym.Env):
    metadata = {"render_modes": ["human"], "render_fps": 33}

    def __init__(self, render_mode=None, simple=False):
        self.game_context_info = binder.init(simple)
        map_width = self.game_context_info["map_width"]
        map_height = self.game_context_info["map_height"]

        monster = spaces.Dict(
            {
                "position": spaces.Box(low=np.array([0, 0]),
                                       high=np.array([map_width, map_height]),
                                       shape=(2,), dtype=np.int32),
                # ghost是否处于frightened状态或pacman是否处于无敌状态，0：no，1：yes
                "status": spaces.Discrete(15 * 33 + 1),
                "move_dir": spaces.Discrete(4),
                "speed": spaces.Discrete(20)
            }
        )
        self.observation_space = spaces.Dict(
            {
                "pacman": monster,
                "ghosts": spaces.Tuple((monster, monster, monster, monster)),
                # 地图块，0：空，1：普通豆子，2：能量豆子，3：墙，4：鬼门
                "map_tiles": spaces.Box(low=0, high=4, shape=(map_width * map_height,), dtype=np.int32),
                # 剩余奖励时间，最多35，最少0
                "bonus_time": spaces.Discrete(36),
                # 剩余生命，最多2，最少0
                "life_remains": spaces.Discrete(3),
                # 是否走到的tile的中心
                "reach_tile": spaces.Discrete(2)
            })

        # RIGHT[0], DOWN[1], LEFT[2], UP[3]
        self.action_space = spaces.Discrete(4)

        assert render_mode is None or render_mode in self.metadata["render_modes"]
        self.render_mode = render_mode

    def _get_obs(self):
        return binder.get_observation()

    def _get_info(self):
        # return {"life_remains": 3, "time_bonus": 300}
        return binder.get_info()

    def reset(
            self,
            seed=None,
            options=None,
    ):
        super().reset(seed=seed)

        binder.reset(seed)

        if self.render_mode == "human":
            self._render_frame()
        return self._get_obs(), self._get_info()

    def step(
            self, action
    ):
        reward, terminated = binder.update(action)
        observation = self._get_obs()
        info = self._get_info()
        if self.render_mode == "human":
            self._render_frame()

        return observation, reward, terminated, False, info

    # ! return Nothing!
    def render(self):
        self._render_frame()
        return

    def _render_frame(self):
        binder.render()

    def close(self):
        binder.quit()
