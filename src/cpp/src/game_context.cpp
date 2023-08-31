#include "game_context.hpp"

#include "context.hpp"

GameContext::GameContext() {
    gameMap.reset(new Map(Map::GenerateMap(beanCount_), {MapWidth, MapHeight}));
    beanLeft_ = beanCount_;

    auto& ctx = Context::GetInstance();
    auto tilesheet = ctx.GetTextureManager().FindTilesheet(TilesheetName);

    Ghost::InitAiMap();

    monsters.emplace_back(new Pacman{
        {tilesheet->Get(static_cast<int>(ImageTileType::Pacman), 0),
         tilesheet->Get(static_cast<int>(ImageTileType::PacmanEat), 0)}});
    monsters.emplace_back(
        new Ghost{{tilesheet->Get(static_cast<int>(ImageTileType::Ghost), 0)},
                  "Blinky",
                  gameMap->NearestAccessibleTile({MapWidth, -1})});
    monsters.emplace_back(
        new Ghost{{tilesheet->Get(static_cast<int>(ImageTileType::Ghost), 0)},
                  "Pinky",
                  gameMap->NearestAccessibleTile({-1, -1})});
    monsters.emplace_back(
        new Ghost{{tilesheet->Get(static_cast<int>(ImageTileType::Ghost), 0)},
                  "Inky",
                  gameMap->NearestAccessibleTile({MapWidth, MapHeight})});
    monsters.emplace_back(
        new Ghost{{tilesheet->Get(static_cast<int>(ImageTileType::Ghost), 0)},
                  "Clyde",
                  gameMap->NearestAccessibleTile({-1, MapHeight})});
    Ghost* Blinky = dynamic_cast<Ghost*>(monsters[1].get());
    Blinky->joinChasing = true;
    Ghost* Pinky = dynamic_cast<Ghost*>(monsters[1].get());
    Pinky->joinChasing = true;
    controller.reset(new Controller(*dynamic_cast<Pacman*>(monsters[0].get())));

    winImage = ctx.GetTextureManager().Find("Win");
    gameoverImage = ctx.GetTextureManager().Find("Gameover");

    initEasterEggInfo();
}

void GameContext::dealCollideWithMap(Monster& monster) {
    Rect monsterRect(monster.GetPosition(), {TileSize, TileSize});
    auto monsterCenter = monsterRect.Center();
    int cx = monsterCenter.x / TileSize;
    int cy = monsterCenter.y / TileSize;
    for (int dx = -1; dx <= 1; dx++) {
        for (int dy = -1; dy <= 1; dy++) {
            auto x = dx + cx, y = dy + cy;
            if (x < 0 || x > MapWidth || y < 0 || y > MapHeight) {
                continue;
            }
            Rect tileRect(Vector2(x * TileSize, y * TileSize),
                          {TileSize, TileSize});
            if (gameMap->GetTile(x, y).type == Tile::Type::Wall &&
                tileRect.IsIntersect(monsterRect)) {
                monsterCenter = monsterRect.Center();
                auto tileCenter = tileRect.Center();
                auto dir = monsterCenter - tileCenter;
                auto maxIntersectV =
                    (monsterRect._.size + tileRect._.size) / 2.0;
                auto intersectDepth =
                    maxIntersectV - Vector2(std::abs(dir.x), std ::abs(dir.y));
                auto seperate =
                    (-monster.GetVelocity()).Normalize() * intersectDepth;
                monster.Move(seperate);
                monsterRect = Rect(monster.GetPosition(), {TileSize, TileSize});
            }
        }
    }
}

void GameContext::Update() {
    auto nowTime = std::chrono::system_clock::now();
    // 从游戏开局到现在的持续时间
    std::chrono::duration<double> globalElapsedDuration = nowTime - globalTime_;
    globalElapsed = globalElapsedDuration.count();
    if (GameState::Gaming == state) {
        std::chrono::duration<double> FrameElapsedDuration =
            nowTime - frameTime_;
        float frameElapsed = FrameElapsedDuration.count();
        frameTime_ = nowTime;
        auto* pacman = dynamic_cast<Pacman*>(monsters[0].get());
        if (pacman->invincibleTime > 0) {
            // pacman处于无敌状态，需要更新计时
            pacman->invincibleTime =
                std::max(0.0f, pacman->invincibleTime - frameElapsed);
        }

        if (energizedTime_ > 0) {
            // 如果充能豆子效果还在，计时不更新，ghost保持frightened状态
            for (int i = 1; i < monsters.size(); i++) {
                Ghost* ghost = dynamic_cast<Ghost*>(monsters[i].get());
                if (ghost->frightenedTime > 0) {
                    ghost->frightenedTime =
                        std::max(0.0f, ghost->frightenedTime - frameElapsed);
                }
            }
            // 减去globalElapsed
            energizedTime_ = std::max(0.0f, energizedTime_ - frameElapsed);
            if (energizedTime_ < std::numeric_limits<float>::epsilon()) {
                multiKillReward_ = MultiKillReward;
            }
        } else {
            // 正常模式
            normalRunningElapsed += frameElapsed;
            auto epoch = GetElapsedFloor() % 27;
            // 根据计时器更改ghost的状态
            if (epoch < 7) {
                for (int i = 1; i < monsters.size(); i++) {
                    Ghost* ghost = dynamic_cast<Ghost*>(monsters[i].get());
                    if (ghost->mode != Ghost::Mode::Scatter) {
                        ghost->ChangeMode(Ghost::Mode::Scatter);
                    }
                }
            } else {
                for (int i = 1; i < monsters.size(); i++) {
                    Ghost* ghost = dynamic_cast<Ghost*>(monsters[i].get());
                    if (ghost->mode != Ghost::Mode::Chase) {
                        ghost->ChangeMode(Ghost::Mode::Chase);
                    }
                }
            }
        }

        auto beanEaten = GetBeanEaten();
        if (beanEaten > 30) {
            // 吃掉超过30个豆子时，inky加入战斗
            Ghost* inky = dynamic_cast<Ghost*>(monsters[3].get());
            inky->joinChasing = true;
        }
        if (beanEaten > beanCount_ / 3) {
            // 吃掉超过1/3豆子时，clyde加入战斗
            Ghost* clyde = dynamic_cast<Ghost*>(monsters[4].get());
            clyde->joinChasing = true;
        }
        for (auto& monster : monsters) {
            monster->Update();
        }

        for (auto& monster : monsters) {
            dealCollideWithMap(*monster);
        }
        tryCapture();
        tryEatBean();
        if (beanLeft_ == 0) {
            state = GameState::Win;
            Won = true;
        }
        updateGameInfoText();
        TryEasterEgg();
    }
}

void GameContext::TryEasterEgg() {
    Pacman* pacman = dynamic_cast<Pacman*>(monsters[0].get());
    for (int i = 1; i < monsters.size(); i++) {
        Ghost* ghost = dynamic_cast<Ghost*>(monsters[i].get());
        if (pacman->GetMapCorrdinate() == ghost->GetScatterPoint()) {
            if (easterEggInfo[i - 1].Check(scancodeQueue_)) {
                pacman->invincibleTime = InvinciateTime;
            }
        }
    }
}

void GameContext::initEasterEggInfo() {
    auto& ctx = Context::GetInstance();
    for (int i = 0; i < 4; i++) {
        auto text = ctx.GenerateTextTexture(std::string(testerList[i]),
                                            {255, 215, 0, 255}, TextWidth[i]);
        easterEggInfo[i] = EasterEgg(i, text);
    }
}

void GameContext::newGame() {
    state = Gaming;
    globalElapsed = 0;
    normalRunningElapsed = 0;
    globalTime_ = std::chrono::system_clock::now();
    frameTime_ = globalTime_;
    energizedTime_ = 0;
    lifeRemaining_ = RemainingLifeCount;
    multiKillReward_ = MultiKillReward;
    score_ = 0;
    modeCount_ = 0;
    // debugMode = false;
    gameMap.reset(new Map(Map::GenerateMap(beanCount_), {MapWidth, MapHeight}));
    beanLeft_ = beanCount_;

    monsters[0]->Reset();
    monsters[1]->Reset();
    monsters[2]->Reset();
    monsters[3]->Reset();
    monsters[4]->Reset();

    // 重置彩蛋
    for (auto& egg : easterEggInfo) {
        egg.show = false;
    }
}

void GameContext::tryCapture() {
    Pacman* pacman = dynamic_cast<Pacman*>(monsters[0].get());
    auto pacmanRect = pacman->GetRect();
    for (int i = 1; i < monsters.size(); i++) {
        Ghost* ghost = dynamic_cast<Ghost*>(monsters[i].get());
        auto ghostRect = ghost->GetRect();
        if (pacmanRect.IsIntersect(ghostRect)) {
            if (ghost->IsFrightened()) {
                std::cout << ghost->name << " is catched" << std::endl;
                ghost->Reset();
                score_ += multiKillReward_;
                multiKillReward_ += MultiKillReward;
            } else {
                if (!(DebugMode || pacman->invincibleTime > 0.0f)) {
                    if (lifeRemaining_ <= 0) {
                        state = GameState::Gameover;
                        GameIsOver = true;
                    } else {
                        lifeRemaining_--;
                        pacman->Reset();
                        pacman->invincibleTime = InvinciateTime;
                    }
                }
            }
        }
    }
}

void GameContext::tryEatBean() {
    auto& pacman = controller->pacman;
    auto pacmanCor = pacman.GetMapCorrdinate();
    bool reach = pacman.ReachTheTile(0.6, 0);
    Tile& tile = gameMap->GetTile(pacmanCor);
    if (reach) {
        switch (tile.type) {
            case Tile::Type::PowerBean:
                energizedTime_ = FrightenedTime;
                for (int i = 1; i < monsters.size(); i++) {
                    Ghost* ghost = dynamic_cast<Ghost*>(monsters[i].get());
                    ghost->ChangeMode(Ghost::Mode::Frightened);
                }
                // ?
                // 这里暂时不重置multiKillReward_，即充能时间被续上时，连杀奖励更高
                // multiKillReward_  = MultiKillReward;
                // 不break，继续执行Bean的内容
            case Tile::Type::Bean:
                tile.type = Tile::Type::Empty;
                score_ += BeanScore;
                beanLeft_--;
                break;
            default:
                break;
        }
        // if (tile.type == Tile::Type::Bean) {
        // }
        // if (tile.type == Tile::Type::PowerBean) {
        //     tile.type = Tile::Type::Empty;
        //     score_ += 10;
        //     beanLeft_--;
        // }
    }
}
