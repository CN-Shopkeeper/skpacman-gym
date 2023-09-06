import math

import numpy as np
import queue
from gymnasium import ObservationWrapper
from gymnasium.spaces import Box


class GreedyObservation(ObservationWrapper):
    def __init__(self, env, simple=False):
        super().__init__(env)
        self.dir_to_coor = [[1, 0], [0, 1], [-1, 0], [0, -1]]
        self.game_ctx_info = env.get_wrapper_attr('game_context_info')
        self.low = []
        self.high = []
        self.length = 0

        space = env.observation_space

        key = ""

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

        key = "map_tiles"
        map_tiles = space.get(key)
        if map_tiles is None:
            raise Exception("Observation_space lack '{}', do you filter it with FilterObservation?".format(key))

        # surroundings
        self.low.extend([0, 0, 0, 0])
        self.high.extend([3, 3, 3, 3])
        self.length += 4

        self.observation_space = Box(shape=(self.length,), low=np.array(self.low), high=np.array(self.high),
                                     dtype=np.int32)

    def observation(self, obs):
        obs_ = []
        obs_.append(obs["reach_tile"])

        pacman = obs["pacman"]
        pacman_pos = pacman["position"]
        ghost_status = obs["ghosts"][0]["status"]

        map_tiles = obs["map_tiles"]
        map_width = self.game_ctx_info["map_width"]

        pacman_surroundings = [0, 0, 0, 0]

        # 贪心算法，远见函数
        max_step = 15
        for dir in range(4):
            coor = self.dir_to_coor[dir]
            # 如果是墙，直接赋值为-1
            if map_tiles[pacman_pos[0] + coor[0] + (pacman_pos[1] + coor[1]) * map_width] in [3, 4]:
                pacman_surroundings[dir] = -1
                continue
            max_weight = 0
            visited: set = set()
            que = queue.Queue()
            visited.add((pacman_pos[0], pacman_pos[1]))
            que.put((pacman_pos[0] + coor[0], pacman_pos[1] + coor[1], 1, 0))
            while not que.empty():
                x, y, step, bean_weight = que.get(block=False)
                tile_type = map_tiles[x + y * map_width]
                if tile_type in [1, 2]:
                    bean_weight += math.pow(max_step - step + 1, 3)
                    if tile_type == 2:
                        frame_to_power_bean = step * self.game_ctx_info["tile_size"] / pacman["speed"]
                        # 如果到下个能量豆之前还有剩余的时间，则要惩罚
                        if frame_to_power_bean < ghost_status:
                            bean_weight = max(bean_weight - math.pow(max_step, 3), bean_weight / 2)
                        else:
                            bean_weight += math.pow(max_step, 3)

                    max_weight = max(max_weight, bean_weight)
                visited.add((x, y))
                if step < max_step:
                    for dir_ in range(4):
                        coor_ = self.dir_to_coor[dir_]
                        dx = x + coor_[0]
                        dy = y + coor_[1]
                        if (dx, dy) not in visited:
                            tile_type_ = map_tiles[dx + dy * map_width]
                            if tile_type_ < 3:
                                que.put((dx, dy, step + 1, bean_weight))
            pacman_surroundings[dir] = max_weight

        sorted_indices = sorted(range(len(pacman_surroundings)), key=lambda i: pacman_surroundings[i])

        weights = [0, 0, 0, 0]
        for i in range(4):
            weights[sorted_indices[i]] = i

        for i in range(1, 4):
            if pacman_surroundings[sorted_indices[i]] == pacman_surroundings[sorted_indices[i - 1]]:
                sub_value = weights[sorted_indices[i]] - weights[sorted_indices[i - 1]]
                weights[sorted_indices[i]] -= sub_value
                for j in range(i + 1, 4):
                    weights[sorted_indices[j]] -= sub_value

        max_value = np.max(weights)

        if max_value <= 1:
            # 如果最大元素为1，说明可能附近找不到豆子或者豆子已经吃完
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
                    weights[trace_back[4]] += 1
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
        obs_.extend(weights)

        return np.array(obs_, dtype=np.int32)
