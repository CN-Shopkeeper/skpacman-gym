# pacman-SDL2

> :warning: ~~**本游戏的全部售卖收入将用于充值原神！**~~

基于 SDL2 的吃豆人游戏

[点击跳转](https://cn-shopkeeper.github.io/Projects/Pacman-SDL2/Pacman.html)

## 一些功能

1. 墙体碰撞监测

2. 移动意图

3. 随机创造地图(R)

4. 四种 Ghost 的 AI

5. 开发者模式(G)

## 算法依据

[地图生成](https://shaunlebron.github.io/pacman-mazegen/) (简化了的)

[游戏机制](https://gameinternals.com/understanding-pac-man-ghost-behavior)

## WASM

```shell
emcmake cmake -S . -B wasm-build
```

```shell
cmake --build wasm-build
```
