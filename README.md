# skpacman-gym

本项目 Clone 自[pacman-SDL2](https://github.com/CN-Shopkeeper/pacman-SDL2)，目的是将 pacman-SDL2
修改为适配[openai 的 gym 项目](https://gymnasium.farama.org/)的接口规范，并使用强化学习进行训练学习。

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
 ┃   ┃ ┃ ┣ 📂 envs 环境类
 ┃   ┃ ┃   ┣ 📂 lib skpacman-sym源码编译结果
 ┃   ┗ 📂 skpacman_rl - 强化学习代码，使用skpacman_gym的类gym环境
 ┣ 📂 extern - 存放submodule，本项目使用了pybind11
```

## skpacman-gym VS. pacman-SDL2

- [x] 相同的游戏机制

- [x] _使用 pybind11 打包成动态链接库_

  - [x] _在 python 脚本中逐帧控制_
  - [x] _暴露 init、quit、update、reset、render 等接口函数_

- [ ] ~~emscripten(WASM)~~

## 递归克隆这个项目

该项目使用了 pybind11 作为 submodule，请使用以下指令进行克隆：

```shell
git@github.com:CN-Shopkeeper/skpacman-gym.git
```

pybind11 将被克隆在`extern/pybind11`中。

## 编译 skpacman-gym 为.pyd

> :warning: 请使用 MSVC 进行项目编译

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

将以上结果复制到`src/python/skpacman_gym/skpacman_gym/envs/lib`中，供`skpacman_gym`的类 gym 环境调用。

### 不建议使用 MinGW

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

在`src/python/skpacman_gym`处

```commandline
python setup.py sdist

pip install ./dist/skpacman_gym-0.0.1.tar.gz
```

## todo

- [ ] env.action_space
- [ ] env.observation_space
- [ ] 修改 pacman 的源码
