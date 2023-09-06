from .lib import Pacman as ctx
from .lib import current_directory
import numpy as np


def get_observation():
    obs = ctx.get_observation()
    pacman = obs["pacman"]
    pacman["position"] = np.array(pacman["position"], dtype=np.int32)

    ghost1, ghost2, ghost3, ghost4 = obs["ghosts"]
    ghost1["position"] = np.array(ghost1["position"], dtype=np.int32)
    ghost2["position"] = np.array(ghost2["position"], dtype=np.int32)
    ghost3["position"] = np.array(ghost3["position"], dtype=np.int32)
    ghost4["position"] = np.array(ghost4["position"], dtype=np.int32)
    obs["map_tiles"] = np.array(obs["map_tiles"], dtype=np.int32)

    return obs


def get_info():
    return {}


def init(simple=False):
    return ctx.init(current_directory, simple)


def reset(seed=None):
    ctx.reset(seed)
    return


# return reward and terminated
def update(intention_direction: int) -> (int, bool):
    return ctx.update(intention_direction)


def render():
    ctx.render()
    return


def quit():
    ctx.quit()
    return
