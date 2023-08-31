def get_pacman_location():
    return [0, 0]


# todo 与cpp交互
def init():
    return {
        "map_width": 10,
        "map_height": 20
    }


# return reward and terminated
def update(intention_direction) -> (int, bool):
    return 0, False


def render():
    return


def quit():
    return
