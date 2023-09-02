#include "bindings.hpp"

#include <pybind11/stl.h>

py::dict Init() {
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

    py::dict ctxInfo;
    ctxInfo["map_width"] = MapWidth;
    ctxInfo["map_height"] = MapHeight;
    return ctxInfo;
}

void Quit() {
    GameContext::Quit();
    Context::Quit();
    TTF_Quit();
    SDL_Quit();
}

void Reset(std::optional<int> randomSeed) {
    auto& gameCtx = GameContext::GetInstance();
    gameCtx.NewGame(randomSeed);
}

std::tuple<int, bool> Update(int intentionCode) {
    auto& gameCtx = GameContext::GetInstance();
    if (intentionCode != 0) {
        SDL_Event event;
        switch (intentionCode) {
            case 1:
                // 模拟按下键盘上的 'W' 键
                event.type = SDL_KEYDOWN;
                event.key.keysym.sym = SDLK_w;
                event.key.keysym.scancode = SDL_SCANCODE_W;
                SDL_PushEvent(&event);

                // 模拟释放键盘上的 'W' 键
                event.type = SDL_KEYUP;
                event.key.keysym.sym = SDLK_w;
                event.key.keysym.scancode = SDL_SCANCODE_W;
                SDL_PushEvent(&event);
                break;

            case 2:
                // 模拟按下键盘上的 'D' 键
                event.type = SDL_KEYDOWN;
                event.key.keysym.sym = SDLK_d;
                event.key.keysym.scancode = SDL_SCANCODE_D;
                SDL_PushEvent(&event);

                // 模拟释放键盘上的 'D' 键
                event.type = SDL_KEYUP;
                event.key.keysym.sym = SDLK_d;
                event.key.keysym.scancode = SDL_SCANCODE_D;
                SDL_PushEvent(&event);
                break;
            case 3:
                // 模拟按下键盘上的 'S' 键
                event.type = SDL_KEYDOWN;
                event.key.keysym.sym = SDLK_s;
                event.key.keysym.scancode = SDL_SCANCODE_S;
                SDL_PushEvent(&event);

                // 模拟释放键盘上的 'S' 键
                event.type = SDL_KEYUP;
                event.key.keysym.sym = SDLK_s;
                event.key.keysym.scancode = SDL_SCANCODE_S;
                SDL_PushEvent(&event);
                break;
            case 4:
                // 模拟按下键盘上的 'A' 键
                event.type = SDL_KEYDOWN;
                event.key.keysym.sym = SDLK_a;
                event.key.keysym.scancode = SDL_SCANCODE_A;
                SDL_PushEvent(&event);

                // 模拟释放键盘上的 'W' 键
                event.type = SDL_KEYUP;
                event.key.keysym.sym = SDLK_a;
                event.key.keysym.scancode = SDL_SCANCODE_A;
                SDL_PushEvent(&event);
                break;

            default:
                break;
        }
    }
    gameCtx.HandleEvent();
    gameCtx.Update();

    int reward = 0;
    bool terminated = false;

    if (gameCtx.eatABean) {
        reward += 10;
    }
    if (gameCtx.Won) {
        // 应该只触发一次
        gameCtx.Won = false;
        gameCtx.CalculateScore();
        reward += gameCtx.GetTimeBonus();
        reward += gameCtx.GetRemainingLifeBonus();
        terminated = true;
        // todo 记录进排行榜
    }
    if (gameCtx.GameIsOver) {
        // 应该只触发一次
        gameCtx.GameIsOver = false;
        reward = -5000;
        terminated = true;
        // todo 记录进排行榜
    }

    return std::tuple<int, bool>(reward, terminated);
}

// py::dict GetObservation() {}

void Render() {
    auto& ctx = Context::GetInstance();
    auto& renderer = Context::GetInstance().GetRenderer();
    renderer.SetColor(SDL_Color{0, 0, 0, 255});
    renderer.Clear();
    draw();
    renderer.Present();
}

void draw() {
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

PYBIND11_MODULE(Pacman, m) {
    m.doc() = "pybind11 example plugin";  // optional module docstring

    m.def("init", &Init, "Init SDL, Context and GameContext");
    m.def("quit", &Quit, "Quit");
    m.def("reset", &Reset, "Reset with seed", py::arg("seed") = std::nullopt);
    m.def("update", &Update, "Update");
    m.def("render", &Render, "Render");
}