from .lib import Pacman as ctx

def get_pacman_location():
    return [0, 0]


def init():
    return ctx.init()

def reset():
    return


# return reward and terminated
def update(intention_direction) -> (int, bool):
    return 0, False


def render():
    return


def quit():
    ctx.quit()
    return
