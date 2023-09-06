import numpy as np
from gymnasium import ObservationWrapper

from gymnasium.spaces import Box


class SKFlattenObservation(ObservationWrapper):
    def __init__(self, env):
        super().__init__(env)
        self.low = []
        self.high = []
        self.length = 0

        # pacman
        space = env.observation_space

        reach_tile_ = space.get("reach_tile")
        if reach_tile_ is not None:
            self.low.append(0)
            self.high.append(reach_tile_.n)
            self.length += 1

        pacman_ = space.get("pacman")
        if pacman_ is not None:
            self.flatten_monster_space(pacman_)

        ghosts_ = space.get("ghosts")
        if ghosts_ is not None:
            for ghost in ghosts_:
                self.flatten_monster_space(ghost)

        map_tiles_ = space.get("map_tiles")
        if map_tiles_ is not None:
            self.low.extend(map_tiles_.low)
            self.high.extend(map_tiles_.high)
            self.length += np.prod(map_tiles_.shape)

        bonus_time_ = space.get("bonus_time")
        if bonus_time_ is not None:
            self.low.append(0)
            self.high.append(bonus_time_.n)
            self.length += 1

        life_remains_ = space.get("life_remains")
        if life_remains_ is not None:
            self.low.append(0)
            self.high.append(life_remains_.n)
            self.length += 1

        self.observation_space = Box(shape=(self.length,), low=np.array(self.low), high=np.array(self.high),
                                     dtype=np.int32)

    def observation(self, obs):
        obs_ = []
        if "reach_tile" in obs:
            obs_.append(obs["reach_tile"])

        if "pacman" in obs:
            obs_.extend(self.flatten_monster(obs["pacman"]))

        if "ghosts" in obs:
            for ghost in obs["ghosts"]:
                obs_.extend(self.flatten_monster(ghost))

        if "map_tiles" in obs:
            obs_.extend(obs["map_tiles"])

        if "bonus_time" in obs:
            obs_.append(obs["bonus_time"])

        if "life_remains" in obs:
            obs_.append(obs["life_remains"])
        return np.array(obs_, dtype=np.int32)

    def flatten_monster_space(self, monster):
        self.low.extend(monster["position"].low)
        self.high.extend(monster["position"].high)
        self.low.append(0)
        self.high.append(monster["status"].n)
        self.low.append(0)
        self.high.append(monster["move_dir"].n)
        self.low.append(0)
        self.high.append(monster["speed"].n)
        self.length += 5

    def flatten_monster(self, monster):
        obs_ = []
        obs_.extend(monster["position"])
        obs_.append(monster["status"])
        obs_.append(monster["move_dir"])
        obs_.append(monster["speed"])
        return obs_
