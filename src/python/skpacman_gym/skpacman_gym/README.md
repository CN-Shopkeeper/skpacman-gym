# skpacman_gym 的类 gym 环境

|                   |                                                                                                                                                                                                            |
| ----------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Action Space      | `Discrete(4)`                                                                                                                                                                                              |
| Observation Space | `Dict('bonus_time': Discrete(36), 'ghosts': Tuple(Monster, Monster, Monster, Monster), 'life_remains': Discrete(3), 'map_tiles': Box(0, 4, (870,), int32), 'pacman': Monster , 'reach_tile': Discrete(2))` |
| Import            | `gymnasium.make('shopkeeper/skpacman-v0')`                                                                                                                                                                 |

Monster 为

```python
Dict('move_dir': Discrete(4), 'position': Box(0, [30 29], (2,), int32), 'speed': Discrete(20), 'status': Discrete(496))
```

可以指定`simple=True`以开启简单模式（无 Ghost）

```python
env = gym.make('shopkeeper/skpacman-v0', render_mode="human", simple=True)
```

## Actions Spaces

| Value | Meaning |
| ----- | ------- |
| 0     | Right   |
| 1     | Down    |
| 2     | Left    |
| 3     | Up      |

## Observations Space

```python
Dict('bonus_time': Discrete(36), 'ghosts': Tuple(Monster, Monster, Monster, Monster), 'life_remains': Discrete(3), 'map_tiles': Box(0, 4, (870,), int32), 'pacman': Monster , 'reach_tile': Discrete(2))
```

| Key          | Meaning               | Type                                        | Explanation                                                                               |
| ------------ | --------------------- | ------------------------------------------- | ----------------------------------------------------------------------------------------- |
| bonus_time   | 剩余奖励时间          | `Discrete(36)`                              |                                                                                           |
| life_remains | 剩余生命数量          | `Discrete(3)  `                             |                                                                                           |
| reach_tile   | pacman 到达 tile 中心 | `Discrete(2)`                               | 游戏机制为只有到达一个 Tile 的中心部分才可以转向，并且正常情况下一帧最多移动 5/24 个 Tile |
| map_tiles    | 地图                  | `Box(0, 4, (870,), int32)`                  | 30×29，长宽信息可以在`env.get_wrapper_attr('game_context_info')`找到                      |
| ghosts       | 四只 Ghost 的状态     | `Tuple(Monster, Monster, Monster, Monster)` |                                                                                           |
| ghosts       | 四只 Ghost 的状态     | `Monster`                                   |                                                                                           |

Monster 为

```python
Dict('move_dir': Discrete(4), 'position': Box(0, [30 29], (2,), int32), 'speed': Discrete(20), 'status': Discrete(496))
```

| Key      | Meaning  | Type                           | Explanation                                                |
| -------- | -------- | ------------------------------ | ---------------------------------------------------------- |
| move_dir | 移动方向 | `Discrete(4)`                  | 与 Action Space 相符                                       |
| speed    | 移动速度 | `Discrete(20)`                 | 单位为像素                                                 |
| status   | 特殊状态 | `Discrete(496)`                | 单位为帧。其中 pacman 为无敌状态，Ghost 为 Frightened 状态 |
| position | 位置     | `Box(0, [30 29], (2,), int32)` | 单位为 Tile，与 Map_tiles 相符                             |

## Wrappers

| Name                 | import                                                   | Explanation                                                                                                                                                                                                    |
| -------------------- | -------------------------------------------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| SKFlattenObservation | `from skpacman_gym.wrappers import SKFlattenObservation` | 将原先的字典形式的 Observation 拉成一维数组，拉伸的顺序是：`["reach_tile", "pacman", "ghosts", "map_tiles", "bonus_time","life_remains"]`；Monster 的拉伸顺序是：`["position", "status", "move_dir", "speed"]` |
| GreedyObservation    | `from skpacman_gym.wrappers import GreedyObservation`    | (simple 模式下)专门为不会强化学习的宝宝提供的贪心算法策略，使用了强大的 Shopkeeper 的远见函数。Observation 修改为`["reach_tile", "Right Weight", "Down Weight", "Left Weight", "Up Weight"]`，选最大的就完事了 |
