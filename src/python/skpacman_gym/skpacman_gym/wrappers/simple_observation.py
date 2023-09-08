import math

import numpy as np
import queue
from gymnasium import ObservationWrapper
from gymnasium.spaces import Box


class SimpleObservation(ObservationWrapper):
    def __init__(self, env, size=(5, 5)):
        super().__init__(env)
        self.dir_to_coor = [[1, 0], [0, 1], [-1, 0], [0, -1]]
        if size is not None and (size[0] % 2 == 0 or size[1] % 2 == 0):
            raise Exception("The `size` {} you provided are divisible by 2.".format(size))
        self.size = size
        self.game_ctx_info = env.get_wrapper_attr('game_context_info')
        self.low = []
        self.high = []
        self.length = 0

        space = env.observation_space

        key = "reach_tile"
        reach_tile = space.get(key)
        if reach_tile is not None:
            self.low.append(0)
            self.high.append(reach_tile.n)
            self.length += 1
        else:
            raise Exception("Observation_space lack '{}', do you filter it with FilterObservation?".format(key))

        key = "pacman"
        pacman = space.get(key)
        if pacman is None:
            raise Exception("Observation_space lack '{}', do you filter it with FilterObservation?".format(key))

        # status
        self.low.append(0)
        self.high.append(pacman["status"].n)
        self.length += 1

        key = "map_tiles"
        map_tiles = space.get(key)
        if map_tiles is None:
            raise Exception("Observation_space lack '{}', do you filter it with FilterObservation?".format(key))

        # surroundings
        if size is None:
            # 加载全图
            tile_count = self.game_ctx_info["map_width"] * self.game_ctx_info["map_height"]
            self.low.extend([0] * tile_count)
            self.high.extend([3] * tile_count)
            self.length += tile_count
        else:
            # 以pacman为中心
            self.low.extend([0] * np.prod(size))
            self.high.extend([3] * np.prod(size))
            self.length += np.prod(size)

        # deus ex machina
        self.low.append(0)
        self.high.append(3)
        self.length += 1

        self.observation_space = Box(shape=(self.length,), low=np.array(self.low), high=np.array(self.high),
                                     dtype=np.int32)

    def observation(self, obs):
        obs_ = []
        obs_.append(obs["reach_tile"])

        pacman = obs["pacman"]
        pac_x, pac_y = pacman["position"]
        obs_.append(obs["ghosts"][0]["status"])

        map_tiles = obs["map_tiles"]
        # 将ghost door 全都替换为wall
        map_tiles = [3 if x == 4 else x for x in map_tiles]
        map_width = self.game_ctx_info["map_width"]
        map_height = self.game_ctx_info["map_height"]

        if self.size is None:
            obs_.extend(map_tiles)
        else:
            surroundings = [0] * (self.size[0] * self.size[1])
            for i in range(self.size[0]):
                map_x = pac_x + (i - (self.size[0] // 2))
                if map_x < 0 or map_x >= map_width:
                    continue
                for j in range(self.size[1]):
                    map_y = pac_y + (j - (self.size[1] // 2))
                    if map_y < 0 or map_y >= map_height:
                        continue
                    surroundings[i + j * self.size[0]] = map_tiles[map_x + map_y * map_width]

            obs_.extend(surroundings)

        # deus ex machina

        pacman_pos = pacman["position"]
        direction = 0

        list_ = []
        index = 0
        visited: set = set()
        visited.add((pacman_pos[0], pacman_pos[1]))
        for dir in range(4):
            coor = self.dir_to_coor[dir]
            tile_type = map_tiles[pacman_pos[0] + coor[0] + (pacman_pos[1] + coor[1]) * map_width]
            if tile_type < 3:
                list_.append((pacman_pos[0] + coor[0], pacman_pos[1] + coor[1], -1, 0, dir))

        while index < len(list_):
            x, y, pre, step, _ = list_[index]
            visited.add((x, y))
            tile_type = map_tiles[x + y * map_width]
            if tile_type in [1, 2]:
                trace_back = list_[index]
                while trace_back[2] != -1:
                    trace_back = list_[trace_back[2]]
                direction = trace_back[4]
                break
            for dir_ in range(4):
                coor_ = self.dir_to_coor[dir_]
                dx = x + coor_[0]
                dy = y + coor_[1]
                if (dx, dy) not in visited:
                    tile_type_ = map_tiles[dx + dy * map_width]
                    if tile_type_ < 3:
                        list_.append((dx, dy, index, step + 1, dir_))

            index += 1

        obs_.append(direction)

        return np.array(obs_, dtype=np.int32)
