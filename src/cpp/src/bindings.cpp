#include "bindings.hpp"

std::unordered_map<std::string, int> Init(const std::string& baseDir,
                                          bool debug) {
    SDL_Init(SDL_INIT_EVERYTHING);
    TTF_Init();
    Context::Init("Pacman", Vector2{WindowWidth, WindowHeight}, baseDir);
    auto& ctx = Context::GetInstance();
    auto& textureManager = ctx.GetTextureManager();
    textureManager.LoadTilesheet(TilesheetName,
                                 baseDir + "/resources/tilesheet.bmp", KeyColor,
                                 TilesheetCol, TilesheetRow);
    textureManager.Load("Win", baseDir + "/resources/win.bmp", KeyColor);
    textureManager.Load("Gameover", baseDir + "/resources/gameover.bmp",
                        KeyColor);
    GameContext::Init();
    RankingList::Init();

    GameContext::GetInstance().DebugMode = debug;

    // Context::GetInstance().
    GameContext::GetInstance().UpdateGameInfoText();

    std::unordered_map<std::string, int> ctxInfo;
    ctxInfo["map_width"] = MapWidth;
    ctxInfo["map_height"] = MapHeight;
    ctxInfo["tile_size"] = TileSize;
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
    auto& event = gameCtx.GetEvent();
    switch (intentionCode) {
        case 0:
            // 模拟按下键盘上的 'D' 键
            event.type = SDL_KEYDOWN;
            event.key.keysym.sym = SDLK_d;
            event.key.keysym.scancode = SDL_SCANCODE_D;

            // // 模拟释放键盘上的 'D' 键
            // event.type = SDL_KEYUP;
            // event.key.keysym.sym = SDLK_d;
            // event.key.keysym.scancode = SDL_SCANCODE_D;
            break;

        case 1:
            // 模拟按下键盘上的 'S' 键
            event.type = SDL_KEYDOWN;
            event.key.keysym.sym = SDLK_s;
            event.key.keysym.scancode = SDL_SCANCODE_S;

            // // 模拟释放键盘上的 'S' 键
            // event.type = SDL_KEYUP;
            // event.key.keysym.sym = SDLK_s;
            // event.key.keysym.scancode = SDL_SCANCODE_S;
            break;
        case 2:
            // 模拟按下键盘上的 'A' 键
            event.type = SDL_KEYDOWN;
            event.key.keysym.sym = SDLK_a;
            event.key.keysym.scancode = SDL_SCANCODE_A;

            // // 模拟释放键盘上的 'A' 键
            // event.type = SDL_KEYUP;
            // event.key.keysym.sym = SDLK_a;
            // event.key.keysym.scancode = SDL_SCANCODE_A;
            break;
        case 3:
            // 模拟按下键盘上的 'W' 键
            event.type = SDL_KEYDOWN;
            event.key.keysym.sym = SDLK_w;
            event.key.keysym.scancode = SDL_SCANCODE_W;

            // // 模拟释放键盘上的 'W' 键
            // event.type = SDL_KEYUP;
            // event.key.keysym.sym = SDLK_w;
            // event.key.keysym.scancode = SDL_SCANCODE_W;
            break;

        default:
            break;
    }
    gameCtx.HandleEvent();

    SDL_Event outEvent;
    while (SDL_PollEvent(&outEvent)) {
        if (outEvent.type == SDL_QUIT) {
            gameCtx.Exit();
            Quit();
        }
        if (outEvent.type == SDL_KEYDOWN) {
            auto key = outEvent.key.keysym.scancode;
            if (SDL_SCANCODE_P == key) {
                if (gameCtx.state == GameContext::GameState::Paused) {
                    gameCtx.state = GameContext::GameState::Gaming;
                    std::cout << "Gaming" << std::endl;
                } else if (gameCtx.state == GameContext::GameState::Gaming) {
                    gameCtx.state = GameContext::GameState::Paused;
                    std::cout << "Paused" << std::endl;
                }
            }
        }
    }

    gameCtx.Update();

    int reward = 0;
    bool terminated = false;

    if (!gameCtx.DebugMode) {
        reward += gameCtx.captureResult;
    }
    if (gameCtx.eatABean) {
        reward += 10;
    }
    if (gameCtx.Won) {
        // 应该只触发一次
        gameCtx.Won = false;
        gameCtx.CalculateScore();
        reward += gameCtx.GetTimeBonus();
        if (!gameCtx.DebugMode) {
            // debug(simple) 模式下不给剩余生命的奖励分了
            reward += gameCtx.GetRemainingLifeBonus();
        }
        terminated = true;
        std::cout << "Your Score Is " << gameCtx.GetScore() << std::endl;
        RankingList::GetInstance().add("skpacman_rl", gameCtx.GetScore());
    }
    if (gameCtx.GameIsOver) {
        // 应该只触发一次
        gameCtx.GameIsOver = false;
        terminated = true;
        std::cout << "Your Score Is " << gameCtx.GetScore() << std::endl;
        RankingList::GetInstance().add("skpacman_rl", gameCtx.GetScore());
    }

    return std::tuple<int, bool>(reward, terminated);
}

ObservationDict GetObservation() {
    ObservationDict observation;
    auto& gameCtx = GameContext::GetInstance();
    auto pacman = dynamic_cast<Pacman*>(gameCtx.monsters[0].get());
    MonsterDict pacman_;
    auto& position = std::array<int, 2>{pacman->GetMapCorrdinate().x,
                                        pacman->GetMapCorrdinate().y};
    pacman_["position"] = position;
    pacman_["status"] = pacman->invincibleFrame;
    pacman_["move_dir"] = static_cast<int>(pacman->movingDir);
    pacman_["speed"] = static_cast<int>(pacman->speed);
    observation["pacman"] = pacman_;

    std::tuple<MonsterDict, MonsterDict, MonsterDict, MonsterDict> ghosts;
    for (int i = 1; i < gameCtx.monsters.size(); i++) {
        auto ghost = dynamic_cast<Ghost*>(gameCtx.monsters[i].get());
        MonsterDict ghost_;
        auto& position = std::array<int, 2>{ghost->GetMapCorrdinate().x,
                                            ghost->GetMapCorrdinate().y};
        ghost_["position"] = position;
        ghost_["status"] = ghost->frightenedFrame;
        ghost_["move_dir"] = static_cast<int>(ghost->movingDir);
        ghost_["speed"] = static_cast<int>(ghost->speed);
        switch (i - 1) {
            case 0:
                std::get<0>(ghosts) = ghost_;
                break;
            case 1:
                std::get<1>(ghosts) = ghost_;
                break;
            case 2:
                std::get<2>(ghosts) = ghost_;
                break;
            case 3:
                std::get<3>(ghosts) = ghost_;
                break;
            default:
                break;
        }
    }
    observation["ghosts"] = ghosts;
    auto& mapTiles = gameCtx.GetMapTiles();
    observation["map_tiles"] = mapTiles;
    observation["bonus_time"] = gameCtx.GetBonusTime();
    observation["life_remains"] = gameCtx.GetRemainingLife();
    observation["reach_tile"] = pacman->ReachTheTile(0.6f, 0.4f) ? 1 : 0;
    return observation;
}

void Render() {
    auto& ctx = Context::GetInstance();
    auto& renderer = Context::GetInstance().GetRenderer();
    renderer.SetColor(SDL_Color{0, 0, 0, 255});
    renderer.Clear();
    draw();
    Context::TimePoint nowTime = std::chrono::system_clock::now();
    std::chrono::duration<double> frameElapsedDuration =
        nowTime - ctx.lastFrame;
    double frameElapsed = frameElapsedDuration.count() * 1000;
    if (frameElapsed >= FrameTime) {
        renderer.Present();
        ctx.lastFrame = nowTime;
    } else {
        SDL_Delay(FrameTime - frameElapsed);
        renderer.Present();
        ctx.lastFrame = std::chrono::system_clock::now();
    }
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
    // 绘制排行榜
    gameCtx.UpdateRankingListText();

    renderer.DrawTextTexture(*gameCtx.rankingListText,
                             TileSize * MapWidth + TipsWidth, 0);
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

    m.def("init", &Init, "Init SDL, Context and GameContext",
          py::arg("base_dir") = ".", py::arg("debug") = false);
    m.def("quit", &Quit, "Quit");
    m.def("reset", &Reset, "Reset with seed", py::arg("seed") = std::nullopt);
    m.def("update", &Update, "Update");
    m.def("render", &Render, "Render");
    m.def("get_observation", &GetObservation, "Get observation");
}