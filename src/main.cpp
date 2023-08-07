#include <iostream>

#include "consts.hpp"
#include "context.hpp"
#include "game_context.hpp"

void StartUp() {
    Context::Init("Pacman", Vector2{WindowWidth, WindowHeight});
    auto& ctx = Context::GetInstance();
    auto& textureManager = ctx.GetTextureManager();
    textureManager.LoadTilesheet(TilesheetName, "./resources/tilesheet.bmp",
                                 KeyColor, TilesheetCol, TilesheetRow);

    GameContext::Init();
}

void Draw() {
    auto& ctx = Context::GetInstance();
    auto& gameCtx = GameContext::GetInstance();
    auto& textureManager = ctx.GetTextureManager();
    auto& renderer = ctx.GetRenderer();
    auto* tilesheet = textureManager.FindTilesheet(TilesheetName);
    if (!tilesheet) {
        SDL_Log("[FatalError]: can't find tilesheet");
    }
    gameCtx.gameMap->Draw();
    for (auto& monster : gameCtx.monsters) {
        monster->Draw();
    }
}

void ShutDown() {
    GameContext::Quit();
    Context::Quit();
}

void Update() {
    auto& gameCtx = GameContext::GetInstance();
    gameCtx.Update();
}

void Run() {
    auto& gameCtx = GameContext::GetInstance();
    auto& event = gameCtx.GetEvent();
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            gameCtx.Exit();
        }
        gameCtx.HandleEvent();
    }

    auto& renderer = Context::GetInstance().GetRenderer();
    renderer.SetColor(SDL_Color{0, 0, 0, 255});
    renderer.Clear();
    Update();
    Draw();
    renderer.Present();
    SDL_Delay(30);
}

int main(int argc, char** argv) {
    SDL_Init(SDL_INIT_EVERYTHING);
    // TTF_Init();
    StartUp();

#ifdef __EMSCRIPTEN__
#include "emscripten.h"
    emscripten_set_main_loop(Run, 0, 1);
#else
    while (!GameContext::GetInstance().ShouldClose()) {
        Run();
    }
#endif

    ShutDown();
    SDL_Quit();
    return 0;
}