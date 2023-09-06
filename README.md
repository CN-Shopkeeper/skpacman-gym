# skpacman-gym

本项目 Clone 自[pacman-SDL2](https://github.com/CN-Shopkeeper/pacman-SDL2)，目的是将 pacman-SDL2
修改为适配[openai 的 gym 项目](https://gymnasium.farama.org/)的接口规范，并使用强化学习进行训练学习。

[skpacman_gym 的类 gym 环境文档](./src/python/skpacman_gym/skpacman_gym/README.md)

## 项目结构说明

```
📦 skpacman-gym
 ┣ 📂 cmake - cmake配置文件
 ┣ 📂 resources - 资源文件
 ┣ 📂 src
 ┃ ┣ 📂 cpp - skpacman-sym的源码，改自pacman-SDL2
 ┃ ┃ ┣ 📂 include - .hpp文件
 ┃ ┃ ┗ 📂 src - .cpp文件
 ┃ ┗ 📂 python - python的源码
 ┃   ┣ 📂 skpacman_gym - skpacman_gym的类gym环境
 ┃   ┃ ┣ 📂 skpacman_gym
 ┃   ┃ ┃ ┣ 📂 envs - 环境类
 ┃   ┃ ┃ ┃ ┣ 📂 lib - skpacman-sym源码编译结果
 ┃   ┃ ┃ ┣ 📂 wrappers
 ┃   ┗ 📂 skpacman_rl - 强化学习代码，使用skpacman_gym的类gym环境
 ┃     ┣ 📂 greedy - 贪心算法
 ┣ 📂 extern - 存放submodule，本项目使用了pybind11
 ┣ 📂 example - 我的编译、打包结果，基于Win11、MSVC2022 amd64、python3.10（其他平台未测试）
```

本项目主要包含三个子工程：

1. 游戏 skpacman-sym 的工程，修改自 pacman-SDL2。编译这个工程可以得到.pyd 动态链接库，是第二个子工程的依赖
2. skpacman_gym 的类 gym 环境，（应该完全）兼容 OpenAI 的 gym 的接口。通过 setuptools 打包这个项目得到 python 依赖 skpacman_gym，可以在第三个子工程中当作强化学习的环境
3. ~~强化学习的网络与训练环境搭建。我准备暂时就用一个全连接的 DQN。~~太难哩，先跑个没有 ghost 的贪心算法。

## skpacman-gym VS. pacman-SDL2

- [x] 相同的游戏机制

- [x] _使用 pybind11 打包成动态链接库_

  - [x] _在 python 脚本中逐帧控制_
  - [x] _暴露 init、quit、update、reset、render 等接口函数_

- [x] _自定义的 gym 环境_

  - [x] _使用上一条的动态链接库，符合 gym 的接口规范_
  - [x] _通过 setuptools 打包成 python 依赖，或直接安装到你的 python 环境中_

- [ ] _使用 skpacman-gym 进行强化学习训练_

  - [x] _贪心算法_

- [ ] ~~移除了 emscripten(WASM)~~
- [ ] ~~移除了用户键盘输入事件监听~~
- [ ] ~~移除了游戏结束时的交互（消息框与 ID 输入）~~

## 递归克隆这个项目

该项目使用了 pybind11 作为 submodule，请使用以下指令进行克隆：

```shell
git clone --recursive git@github.com:CN-Shopkeeper/skpacman-gym.git
```

pybind11 将被克隆在`extern/pybind11`中。

## 编译 skpacman-gym 为.pyd

> :warning: Windows 下请使用 MSVC 进行项目编译

需要为 Cmake 配置 SDL 与 SDL_TTF 的依赖位置，下面两种方式二选一。

- 使用 vscode 的 cmake 插件

  ```json
  // .vscode/settings.json
  {
    "cmake.configureArgs": [
      "-DSDL2_ROOT=<your SDL path>",
      "-DSDL2_TTF_ROOT=<your SDL_TTF path>"
    ]
  }
  ```

- 或直接从命令行开始

  ```shell
  cmake -S . -B cmake-build -DSDL2_ROOT=<your SDL path> -DSDL2_TTF_ROOT=<your SDL_TTF path>
  cmake --build cmake-build
  ```

生成的结果是

- `resources` 文件夹，包含 pacman 的资源
- `Pacman.cpxxx_xxx_xx.pyd` 供 python 调用的动态链接库
- `SDL2.dll` SDL 的动态链接库
- `SDL2_ttf.dll` SDL_ttf 的动态链接库

可以参考`exmaple/cmake_build`。

将以上结果复制到`src/python/skpacman_gym/skpacman_gym/envs/lib`中，供`skpacman_gym`的类 gym 环境调用。

### 不建议在 Windows 下使用 MinGW

根据`pybind11`的文档说明，最好使用 MSVC 进行编译，我在尝试使用 MinGW 编译后的.pyd 文件时，在 python 脚本中调用报错：

```shell
>>> import example
Traceback (most recent call last):
  File "<stdin>", line 1, in <module>
ImportError: DLL load failed while importing example: The specified module could not be found.
```

与 GitHub 中这个[Issue](https://github.com/pybind/pybind11/issues/2010)的问题一样。

正如这个 Issue 提出的，弥补方式是手动将 MinGW 的 bin 添加到 python 的 .dll 搜寻目录中：

```python
import os

os.add_dll_directory("X:\\XXX\\mingw64\\bin")
import Pacman as ctx
```

## 安装 skpacman-gym 的类 gym 环境

[skpacman_gym 的类 gym 环境文档](./src/python/skpacman_gym/skpacman_gym/README.md)

在`src/python/skpacman_gym`处

- 打包并安装

  ```commandline
  python setup.py sdist

  pip install ./dist/skpacman_gym-0.0.1.tar.gz
  ```

  打包的结果可以参考`exmaple/setuptools`

- 或直接安装

  ```commandline
    pip install .
  ```

如果你是 python3.10，可以直接 pip install `exmaple/setuptools/skpacman_gym-0.0.1.tar.gz`

## 使用 skpacman-gym 进行强化学习训练

暂时还没开始做，目前放了一个使用 skpacman-gym 的例子，该例子中每一帧 pacman 都选取随机的一个 action 进行操作。见`src/python/skpacman_rl/test.py`。
确保你完成上一步 skpacman_gym 的类 gym 环境的安装。

## todo

- 使用 DQN 进行训练
