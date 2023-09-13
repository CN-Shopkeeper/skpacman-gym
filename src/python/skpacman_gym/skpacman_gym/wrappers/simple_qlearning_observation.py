import numpy as np
from gymnasium import ObservationWrapper
from gymnasium.spaces import Box
import queue


class SimpleQLearningObservation(ObservationWrapper):
    def __init__(self, env, performance=False):
        '''
        :param env: 环境
        :param performance: performance为性能模式，默认为False，开启后当且仅当reach_tile == 1时进行实际的observation转换
        '''
        super().__init__(env)
        self.performance = performance
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
        reach_tile = obs["reach_tile"]
        if self.performance and reach_tile == 0:
            return np.array([0] * self.length, dtype=np.int32)
        obs_ = [reach_tile]

        pacman = obs["pacman"]
        pac_x, pac_y = pacman["position"]

        status = obs["ghosts"][0]["status"]

        map_tiles = obs["map_tiles"]
        # 将ghost door 全都替换为wall
        map_tiles = [3 if x == 4 else x for x in map_tiles]
        map_width = self.game_ctx_info["map_width"]

        # power_bean
        power_bean_info = None
        power_beans = [index for index, value in enumerate(map_tiles) if value == 2]
        if len(power_beans) == 0:
            power_bean_info = 0

        # deus ex machina
        pacman_pos = pacman["position"]
        deus_ex_machina_arr = [0] * 4
        deus_ex_machina_max_step = 999

        list_ = []
        index = 0
        visited: set = set()
        visited.add((pacman_pos[0], pacman_pos[1]))
        for direction in range(4):
            coor = self.dir_to_coor[direction]
            tile_type = map_tiles[pacman_pos[0] + coor[0] + (pacman_pos[1] + coor[1]) * map_width]
            if tile_type < 3:
                list_.append((pacman_pos[0] + coor[0], pacman_pos[1] + coor[1], -1, 0, direction))

        while index < len(list_):
            x, y, pre, step, _ = list_[index]
            # 能量豆只检查十步以内的
            if step > 10 and power_bean_info is None:
                power_bean_info = 0
            # 超过机械降神最大步长，且找到最近能量豆的信息，直接返回
            if step > deus_ex_machina_max_step and power_bean_info is not None:
                break
            visited.add((x, y))
            tile_type = map_tiles[x + y * map_width]
            # deus ex machina
            if tile_type in [1, 2]:
                if deus_ex_machina_max_step > step:
                    deus_ex_machina_max_step = step
                    trace_back = list_[index]
                    while trace_back[2] != -1:
                        trace_back = list_[trace_back[2]]
                    deus_ex_machina_arr[trace_back[4]] = 1
            # power bean info
            if tile_type == 2 and power_bean_info is None:
                trace_back = list_[index]
                while trace_back[2] != -1:
                    trace_back = list_[trace_back[2]]
                power_bean_dir = trace_back[4]
                frame_to_go = (self.game_ctx_info["tile_size"] * step) / pacman["speed"]
                if frame_to_go >= status:
                    power_bean_info = power_bean_dir + 1
                else:
                    power_bean_info = -(power_bean_dir + 1)

            for dir_ in range(4):
                coor_ = self.dir_to_coor[dir_]
                dx = x + coor_[0]
                dy = y + coor_[1]
                if (dx, dy) not in visited:
                    tile_type_ = map_tiles[dx + dy * map_width]
                    if tile_type_ < 3:
                        list_.append((dx, dy, index, step + 1, dir_))

            index += 1

        if power_bean_info is None:
            # 手动清除reach_tile==0时没吃到能量豆的情形
            power_bean_info = 0
        obs_.append(power_bean_info)

        # surroundings
        for direction in range(4):
            dir_x, dir_y = self.dir_to_coor[direction]
            obs_.append(map_tiles[pac_x + dir_x + (pac_y + dir_y) * map_width])

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
