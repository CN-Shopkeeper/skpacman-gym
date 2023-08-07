#pragma once

#include <algorithm>
#include <array>
#include <iostream>
#include <memory>
#include <optional>
#include <queue>
#include <random>
#include <stack>
#include <string>
#include <string_view>
#include <unordered_map>

#if __EMSCRIPTEN__
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <emscripten/emscripten.h>
#else
#include "SDL.h"
#include "SDL_ttf.h"
#endif

#include "singlton.hpp"
#include "vmath.hpp"