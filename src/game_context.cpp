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
    if (GameState::Gaming == state) {
        auto nowTime = std::chrono::system_clock::now();
        std::chrono::duration<double> globalElapsedSeconds = nowTime - globalTime_;
        globalElapsed = globalElapsedSeconds.count();
        std::chrono::duration<double> elapsedSeconds =
            nowTime - lastRecordTime_;
        if (energized_) {
            // 如果吃了充能豆子，计时不更新，ghost保持frightened状态
            if (elapsedSeconds.count() >= 15.0f) {
                // 超过15秒后退出
                energized_ = false;
            }
        } else {
            // 否则更新计时器
            elapsed += elapsedSeconds.count();
            lastRecordTime_ = nowTime;
            auto epoch = GetElapsedFloor() % 27;
            // 根据计时器更改ghost的状态
            if (epoch == 0) {
                for (int i = 1; i < monsters.size(); i++) {
                    Ghost* ghost = dynamic_cast<Ghost*>(monsters[i].get());
                    ghost->ChangeMode(Ghost::Mode::Scatter);
                }
            }
            if (epoch == 7) {
                for (int i = 1; i < monsters.size(); i++) {
                    Ghost* ghost = dynamic_cast<Ghost*>(monsters[i].get());
                    ghost->ChangeMode(Ghost::Mode::Chase);
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
        if (energized_) {
            for (int i = 1; i < monsters.size(); i++) {
                Ghost* ghost = dynamic_cast<Ghost*>(monsters[i].get());
                ghost->ChangeMode(Ghost::Mode::Frightened);
            }
        }
        if (beanLeft_ == 0) {
            state = GameState::Win;
        }
        updateGameInfoText();
    }
}

void GameContext::newGame() {
    state = Gaming;
    lastRecordTime_ = std::chrono::system_clock::now();
    globalTime_ = std::chrono::system_clock::now();
    elapsed = 0;
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
                score_ += 200;
            } else {
                state = GameState::Gameover;
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
                energized_ = true;
                // 不break，继续执行Bean的内容
            case Tile::Type::Bean:
                tile.type = Tile::Type::Empty;
                score_ += 10;
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
