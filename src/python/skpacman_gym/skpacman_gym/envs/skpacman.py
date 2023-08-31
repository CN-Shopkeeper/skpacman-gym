import gymnasium as gym
from gymnasium import spaces

import numpy as np

from . import binder


class SKPacmanEnv(gym.Env):
    metadata = {"render_modes": ["human", "rgb_array"]}

    def __init__(self, render_mode=None):
        game_context_info = binder.init()
        self.observation_space = spaces.Dict(
            {"pacman": spaces.Box(low=np.array([0, 0]),
                                  high=np.array([game_context_info["map_width"], game_context_info["map_height"]]),
                                  shape=(2,), dtype=np.float32)})

        # NOOP[0], UP[1], RIGHT[2], DOWN[3], LEFT[4]
        self.action_space = spaces.Discrete(5)

        assert render_mode is None or render_mode in self.metadata["render_modes"]
        self.render_mode = render_mode

    def _get_obs(self):
        return {"agent": binder.get_pacman_location()}

    def _get_info(self):
        return {"life_remains": 3, "time_bonus": 300}

    def reset(
            self,
            seed=None,
            options=None,
    ):
        super().reset(seed=seed)

        if self.render_mode == "human":
            self._render_frame()
        return self._get_obs(), self._get_obs()

    def step(
            self, action
    ):
        reward, terminated = binder.update(action)
        observation = self._get_obs()
        info = self._get_info()
        if self.render_mode == "human":
            self._render_frame()

        return observation, reward, terminated, False, info

    def render(self):
        if self.render_mode == "rgb_array":
            return self._render_frame()

    def _render_frame(self):
        rgb_array = binder.render()

        if self.render_mode == "human":
            return
        else:  # rgb_array
            # return np.transpose(
            #     np.array(pygame.surfarray.pixels3d(canvas)), axes=(1, 0, 2)
            # )
            return rgb_array

    def close(self):
        binder.quit()
