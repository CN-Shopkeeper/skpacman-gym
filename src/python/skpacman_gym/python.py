import time

from skpacman_gym.envs.lib import Pacman as ctx
from skpacman_gym.envs.lib import current_directory
print(current_directory)
ctx.init()
counter=0
while True:
    ctx.update(0)
    ctx.render()
    time.sleep(0.03)
    counter+=1
    if counter>100:
        break
ctx.quit()