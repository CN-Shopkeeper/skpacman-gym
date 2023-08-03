#include <iostream>

#include "consts.hpp"
#include "context.hpp"
#include "game_context.hpp"

void StartUp() {
    Context::Init("Pacman", Vector2{720, 600});
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
    renderer.DrawImage(tilesheet->Get(1, 0), {100, 200},
                       std::optional<Vector2>{Vector2{36, 36}});
}

void ShutDown() {
    GameContext::Quit();
    Context::Quit();
}

void Update() {}

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
    Draw();
    renderer.Present();
    SDL_Delay(30);
}

int main(int argc, char** argv) {
    SDL_Init(SDL_INIT_EVERYTHING);
    // TTF_Init();
    StartUp();

    while (!GameContext::GetInstance().ShouldClose()) {
        Run();
    }

    ShutDown();
    SDL_Quit();
    return 0;
}