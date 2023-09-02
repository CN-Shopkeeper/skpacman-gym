from .lib import Pacman as ctx

def get_observation():
    return {}

def get_info():
    return {}

def init():
    return ctx.init()


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
