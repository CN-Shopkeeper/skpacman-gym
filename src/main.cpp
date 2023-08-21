#include <iostream>

#include "consts.hpp"
#include "context.hpp"
#include "game_context.hpp"
#include "text_input.hpp"

void StartUp() {
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
    int textHeightOffset = 0;
    // 绘制提示
    auto tips = ctx.tips.get();
    renderer.DrawTextTexture(*tips, TileSize * MapWidth, 0);
    textHeightOffset += tips->rect.h + TileSize;
    // 绘制游戏信息
    renderer.DrawTextTexture(*gameCtx.gameInfoText, TileSize * MapWidth,
                             textHeightOffset);
    textHeightOffset += gameCtx.gameInfoText->rect.h;
    // 绘制生命数量
    auto pacmanImage =
        tilesheet->Get(static_cast<int>(ImageTileType::PacmanEat), 0);
    for (int i = 0; i < gameCtx.GetRemainingLife(); i++) {
        renderer.DrawImage(
            pacmanImage,
            {static_cast<float>(TileSize * MapWidth + i * TileSize),
             static_cast<float>(textHeightOffset)},
            {Scale, Scale}, 0.0f);
    }
    textHeightOffset += TileSize * 2;
    // 绘制debug信息
    if (gameCtx.DebugMode) {
        gameCtx.UpdateDebugText();
        renderer.DrawTextTexture(*gameCtx.debugText, TileSize * MapWidth,
                                 textHeightOffset);
        textHeightOffset += gameCtx.debugText->rect.h + TileSize;
    }
    // 绘制用户ID
    if (ctx.playerIdHandler.canInput) {
        auto text = ctx.GenerateTextTexture(
            "Your ID is:\n(" +
            std::to_string(ctx.playerIdHandler.GetContent().length()) +
            "/10)\n" + ctx.playerIdHandler.GetContent() +
            "\nEsc to Cancel\nEnter to Confirm");
        renderer.DrawTextTexture(*text, TileSize * MapWidth, textHeightOffset);
    }
    // 绘制排行榜
    auto text = ctx.GenerateTextTexture(RankingList::GetInstance().ToString());

    renderer.DrawTextTexture(*text, TileSize * MapWidth + TipsWidth, 0);
    //  绘制彩蛋
    for (auto& easterEgg : gameCtx.easterEggInfo) {
        if (easterEgg.show) {
            renderer.DrawTextTexture(*easterEgg.text, easterEgg.position.x,
                                     easterEgg.position.y);
        }
    }
    if (gameCtx.state == GameContext::GameState::Win) {
        renderer.DrawTexture(*gameCtx.winImage, SDL_Rect{0, 0, 256, 256},
                             (WindowWidth - 256) / 2, (WindowHeight - 256) / 2);
    }
    if (gameCtx.state == GameContext::GameState::Gameover) {
        renderer.DrawTexture(*gameCtx.gameoverImage, SDL_Rect{0, 0, 256, 256},
                             (WindowWidth - 256) / 2, (WindowHeight - 256) / 2);
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
    auto& ctx = Context::GetInstance();
    auto& gameCtx = GameContext::GetInstance();
    auto& event = gameCtx.GetEvent();
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            gameCtx.Exit();
        }
        if (ctx.playerIdHandler.canInput) {
            ctx.playerIdHandler.HandleEvent(event);
            if (ctx.playerIdHandler.finished) {
                RankingList::GetInstance().add(ctx.playerIdHandler.GetContent(),
                                               gameCtx.GetScore());
                ctx.playerIdHandler.finished = false;
            }
        } else {
            gameCtx.HandleEvent();
        }
    }

    auto& renderer = Context::GetInstance().GetRenderer();
    renderer.SetColor(SDL_Color{0, 0, 0, 255});
    renderer.Clear();
    Update();
    Draw();
    renderer.Present();

    if (gameCtx.WonMessage) {
        // 应该只触发一次
        gameCtx.WonMessage = false;
        gameCtx.CalculateScore();
        auto result = ShowMessageBox(
            "You Win!", ("Your Socre Is " + std::to_string(gameCtx.GetScore()) +
                         "\n是否录入进排行榜?\n如是, 请输入你的ID")
                            .c_str());
        if (MessageBoxResult::Yes == result) {
            SDL_StartTextInput();
            ctx.playerIdHandler.canInput = true;
        }
    }
    SDL_Delay(30);
}

int main(int argc, char** argv) {
    SDL_Init(SDL_INIT_EVERYTHING);
    TTF_Init();
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
    TTF_Quit();
    SDL_Quit();
    return 0;
}