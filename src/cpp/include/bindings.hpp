#include <pybind11/pybind11.h>

#include "consts.hpp"
#include "context.hpp"
#include "game_context.hpp"

namespace py = pybind11;

void Init() {
    SDL_Init(SDL_INIT_EVERYTHING);
    TTF_Init();
    Context::Init("Pacman", Vector2{WindowWidth, WindowHeight});
    auto& ctx = Context::GetInstance();
    auto& textureManager = ctx.GetTextureManager();
    textureManager.LoadTilesheet(TilesheetName, "./resources/tilesheet.bmp",
                                 KeyColor, TilesheetCol, TilesheetRow);
    textureManager.Load("Win", "./resources/win.bmp", KeyColor);
    textureManager.Load("Gameover", "./resources/gameover.bmp", KeyColor);
    GameContext::Init();
    RankingList::Init();
}

void Quit() {
    GameContext::Quit();
    Context::Quit();
    TTF_Quit();
    SDL_Quit();
}
