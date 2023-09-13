import numpy as np
from gymnasium import ObservationWrapper
from gymnasium.spaces import Box
import queue


class SimpleQLearningObservation(ObservationWrapper):
    def __init__(self, env):
        super().__init__(env)
        self.dir_to_coor = [[1, 0], [0, 1], [-1, 0], [0, -1]]
        self.game_ctx_info = env.get_wrapper_attr('game_context_info')
        self.map_width = self.game_ctx_info["map_width"]
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
        self.low.append(-4)
        self.high.append(4)
        self.length += 1

        key = "map_tiles"
        map_tiles = space.get(key)
        if map_tiles is None:
            raise Exception("Observation_space lack '{}', do you filter it with FilterObservation?".format(key))

        # surroundings
        self.low.extend([0] * 4)
        self.high.extend([3] * 4)
        self.length += 4

        # deus ex machina
        self.low.extend([0] * 4)
        self.high.extend([3] * 4)
        self.length += 4

        self.observation_space = Box(shape=(self.length,), low=np.array(self.low), high=np.array(self.high),
                                     dtype=np.int32)

    def observation(self, obs):
        obs_ = []
        obs_.append(obs["reach_tile"])

        pacman = obs["pacman"]
        pac_x, pac_y = pacman["position"]

        status = obs["ghosts"][0]["status"]

        map_tiles = obs["map_tiles"]
        # 将ghost door 全都替换为wall
        map_tiles = [3 if x == 4 else x for x in map_tiles]
        map_width = self.game_ctx_info["map_width"]

        power_beans = [index for index, value in enumerate(map_tiles) if value == 2]
        if len(power_beans) > 0:
            step = 999
            dir = 0
            for power_bean in power_beans:
                p_x_ = power_bean % map_width
                p_y_ = power_bean // map_width
                step_, dir_ = self._shortest_path((p_x_, p_y_), (pac_x, pac_y), map_tiles)
                dir_ = (dir_ + 2) % 4
                if step_ < step:
                    step = step_
                    dir = dir_
            frame_to_go = (self.game_ctx_info["tile_size"] * step) / pacman["speed"]
            if frame_to_go >= status:
                obs_.append(dir + 1)
            else:
                obs_.append(-(dir + 1))
        else:
            # 没有能量豆了，直接返回0
            obs_.append(0)

        for dir in range(4):
            dir_x, dir_y = self.dir_to_coor[dir]
            obs_.append(map_tiles[pac_x + dir_x + (pac_y + dir_y) * map_width])

        # deus ex machina

        pacman_pos = pacman["position"]
        deus_ex_machina_arr = [0] * 4
        max_step = 999

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
            # 超过最大步长，直接返回
            if step > max_step:
                break
            visited.add((x, y))
            tile_type = map_tiles[x + y * map_width]
            if tile_type in [1, 2]:
                max_step = step
                trace_back = list_[index]
                while trace_back[2] != -1:
                    trace_back = list_[trace_back[2]]
                deus_ex_machina_arr[trace_back[4]] = 1

            for dir_ in range(4):
                coor_ = self.dir_to_coor[dir_]
                dx = x + coor_[0]
                dy = y + coor_[1]
                if (dx, dy) not in visited:
                    tile_type_ = map_tiles[dx + dy * map_width]
                    if tile_type_ < 3:
                        list_.append((dx, dy, index, step + 1, dir_))

            index += 1

        obs_.extend(deus_ex_machina_arr)

        return np.array(obs_, dtype=np.int32)

    def _get_tile_type(self, x, y, map_tile):
        return map_tile[x + y * self.map_width]

    def _shortest_path(self, source, target, map_tile) -> (int, int):
        if source == target:
            return 0, 0
        sx, sy = source
        tx, ty = target
        s_type = self._get_tile_type(sx, sy, map_tile)
        t_type = self._get_tile_type(tx, ty, map_tile)

        if s_type >= 3 or t_type >= 3:
            return 999, 0

        visited: set = set()
        visited.add((sx, sy))
        que = queue.Queue()
        for d in range(4):
            cx, cy = self.dir_to_coor[d]
            tile_type = self._get_tile_type(cx + sx, cy + sy, map_tile)
            if tile_type < 3:
                que.put((cx + sx, cy + sy, 1, d))

        while not que.empty():
            now_x, now_y, step, now_dir = que.get(block=False)
            visited.add((now_x, now_y))
            if now_x == tx and now_y == ty:
                return step, now_dir

            for dir_ in range(4):
                cx, cy = self.dir_to_coor[dir_]
                dx = now_x + cx
                dy = now_y + cy
                if (dx, dy) not in visited:
                    tile_type_ = self._get_tile_type(dx, dy, map_tile)
                    if tile_type_ < 3:
                        que.put((dx, dy, step + 1, dir_))
