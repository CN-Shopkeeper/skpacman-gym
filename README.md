# skpacman-gym

本项目 Clone 自[pacman-SDL2](https://github.com/CN-Shopkeeper/pacman-SDL2)，目的是将 pacman-SDL2
修改为适配[openai 的 gym 项目](https://gymnasium.farama.org/)的接口规范，并使用强化学习进行训练学习。

## skpacman-gym VS. pacman-SDL2

- [x] 相同的游戏机制

- [ ] ~~emscripten(WASM)~~

## 安装skpacman-gym的类gym环境

在`src/python/skpacman_gym`处

```commandline
python setup.py sdist  

pip install ./dist/skpacman_gym-0.0.1.tar.gz 
```

## todo

- [ ] env.action_space
- [ ] env.observation_space
