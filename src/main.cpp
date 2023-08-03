#include <iostream>

#include "context.hpp"
#include "game_context.hpp"

void StartUp() {
    Context::Init("Pacman", Vector2{720, 600});
    GameContext::Init();
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
    renderer.SetColor(SDL_Color{110, 110, 110, 255});
    renderer.Clear();
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