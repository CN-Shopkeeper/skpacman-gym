#include "monster.hpp"

#include "game_context.hpp"

MapCoordinate Monster::DirectionToCoordinate(Direction& direction) {
    switch (direction) {
        case Direction::Up:
            return {0, -1};
        case Direction::Down:
            return {0, 1};
        case Direction::Left:
            return {-1, 0};
        default:
        case Direction::Right:
            return {1, 0};
    }
}

void Monster::Draw() {
    Vector2 scale;
    float rotation = 0;
    scale.x = movingDir == Direction::Left ? -1 : 1;
    scale.y = 1;
    if (movingDir == Direction::Up) {
        rotation = -90;
    }
    if (movingDir == Direction::Down) {
        rotation = 90;
    }

    auto& renderer = Context::GetInstance().GetRenderer();
    renderer.DrawImage(GetImage(), position_, scale * Scale, rotation);

    if (GameContext::GetInstance().DebugMode) {
        Debug();
    }
}

void Monster::doUpdate() {
    if (movingDir == Direction::Left) {
        Move({-speed, 0});
    }
    if (movingDir == Direction::Right) {
        Move({speed, 0});
    }
    if (movingDir == Direction::Up) {
        Move({0, -speed});
    }
    if (movingDir == Direction::Down) {
        Move({0, speed});
    }
}

bool Monster::ReachTheTile(float threshold, float offset) {
    auto& gameCtx = GameContext::GetInstance();
    auto& gameMap = gameCtx.gameMap;
    Rect monsterRect(GetPosition(), {TileSize, TileSize});
    auto monsterCenter = monsterRect.Center();
    auto tileCor = GetMapCorrdinate();
    int tileX = tileCor.x;
    int tileY = tileCor.y;
    Rect tile = Rect{Vector2{tileX * TileSize * 1.f, tileY * TileSize * 1.f},
                     {TileSize, TileSize}};
    auto tileCenter = tile.Center();
    auto diff = tileCenter - monsterCenter;
    bool reach = false;
    // 1.5是Right=0，Down=1和Left=2，Up=3的分界线
    const double offset_ =
        1.5 - static_cast<int>(movingDir) > 0 ? -offset : offset;
    if (movingDir == Monster::Direction::Left ||
        movingDir == Monster::Direction::Right) {
        if (diff.x >= speed * (offset_ - threshold) &&
            diff.x <= speed * (offset_ + threshold)) {
            reach = true;
        }
    }
    if (movingDir == Monster ::Direction::Up ||
        movingDir == Monster ::Direction::Down) {
        if (diff.y >= speed * (offset_ - threshold) &&
            diff.y <= speed * (offset_ + threshold)) {
            reach = true;
        }
    }
    return reach;
}

// 可以在转向时对位置进行一定的容差
// 转向时，将转向前的方向对齐到网格上（阈值为速度的-20%~100%）
void Monster::Update() {
    if (intentionDir == movingDir) {
        Monster::doUpdate();
    } else if (isTurnBack()) {
        movingDir = intentionDir;
        Monster::doUpdate();
    } else {
        auto& gameCtx = GameContext::GetInstance();
        auto& gameMap = gameCtx.gameMap;
        auto monsterCenter = GetRect().Center();
        MapCoordinate tileCoordinate = GetMapCorrdinate();
        auto tileX = tileCoordinate.x;
        auto tileY = tileCoordinate.y;
        Rect tile = Rect{Vector2{tileCoordinate.x * TileSize * 1.f,
                                 tileCoordinate.y * TileSize * 1.f},
                         {TileSize, TileSize}};
        auto tileCenter = tile.Center();
        auto diff = tileCenter - monsterCenter;
        bool should = false;
        bool reach = ReachTheTile(0.6, 0.4);

        if (reach) {
            const int dirOffset =
                1.5 - static_cast<int>(intentionDir) > 0 ? 1 : -1;
            if (movingDir == Monster::Direction::Left ||
                movingDir == Monster::Direction::Right) {
                if (gameMap->IsInside(tileX, tileY + dirOffset)) {
                    should = isAccessible(
                        *gameMap, {static_cast<int>(tileX),
                                   static_cast<int>(tileY + dirOffset)});
                }
            }
            if (movingDir == Monster ::Direction::Up ||
                movingDir == Monster ::Direction::Down) {
                if (gameMap->IsInside(tileX + dirOffset, tileY)) {
                    should = isAccessible(*gameMap,
                                          {static_cast<int>(tileX + dirOffset),
                                           static_cast<int>(tileY)});
                }
            }
        }

        if (!should) {
            Monster::doUpdate();
        } else {
            Move(diff);
            movingDir = intentionDir;
        }
    }
}

bool Monster::isTurnBack() const {
    return (4 + static_cast<int>(intentionDir) - static_cast<int>(movingDir)) %
               4 ==
           2;
}

Image& Pacman::GetImage() {
    auto& gameCtx = GameContext::GetInstance();
    int index = std::fmod(gameCtx.globalElapsed, 0.4f) < 0.2 ? 0 : 1;
    return images.at(index);
}

void Pacman::Update() {
    auto& gameCtx = GameContext::GetInstance();
    auto& image = GetImage();
    if (invincibleTime > 0) {
        image.color = std::fmod(gameCtx.globalElapsed, 0.2f) < 0.1
                          ? InvincibleColor
                          : WhiteColor;
        speed = 10;
    } else {
        image.color = WhiteColor;
        speed = 5;
    }
    Monster::Update();
}

void Ghost::Update() {
    auto& gameCtx = GameContext::GetInstance();
    auto& pacman = gameCtx.controller->pacman;
    // 更新颜色
    // 如果害怕效果时间还剩3秒，应当闪烁提示
    if (frightenedTime > 0) {
        if (frightenedTime <= 3.0f) {
            if (std::fmod(gameCtx.globalElapsed, 0.4f) < 0.2) {
                GetImage().color = GetColor();
            } else {
                GetImage().color = FrightenedColor;
            }
        } else {
            GetImage().color = FrightenedColor;
        }
    }
    if (!joinChasing) {
        intentionDir = aiMap_["Waiting"](pacman, *this);
    } else {
        auto cor = GetMapCorrdinate();
        auto leftD = LeftDirection();
        auto rightD = RightDirection();
        auto left = DirectionToCoordinate(leftD) + cor;
        auto right = DirectionToCoordinate(rightD) + cor;
        if (cor.x == checkPoint_.x && cor.y == checkPoint_.y) {
            // 刚过检查点，
            // 如果意图方向面前的tile不可到达，则尝试直行、拐弯或反向。这个情况可能会在以下两种情况遇到：
            // 1.
            // 在开发者模式，当鬼怪和吃豆人在倒L形状路口接触时，会直接停住不动弹，理由是当没有路径时默认返回向上
            // 2. 在Scatter模式下，绕着自己的ScatterPoint的墙壁移动时，会卡住
            auto directionCor = DirectionToCoordinate(intentionDir);
            if (!gameCtx.gameMap
                     ->GetTile(cor.x + directionCor.x, cor.y + directionCor.y)
                     .IsGhostAccessible()) {
                auto forward = DirectionToCoordinate(movingDir) + cor;
                if (gameCtx.gameMap->GetTile(forward.x, forward.y)
                        .IsGhostAccessible()) {
                    intentionDir = movingDir;
                } else if (gameCtx.gameMap->GetTile(left.x, left.y)
                               .IsGhostAccessible()) {
                    intentionDir = leftD;
                } else if (gameCtx.gameMap->GetTile(right.x, right.y)
                               .IsGhostAccessible()) {
                    intentionDir = rightD;
                } else {
                    intentionDir = BackDirection();
                }
            }
        } else {
            // 检查是否在路口
            auto& leftTile = gameCtx.gameMap->GetTile(left.x, left.y);
            auto& rightTile = gameCtx.gameMap->GetTile(right.x, right.y);
            if (leftTile.IsGhostAccessible() || rightTile.IsGhostAccessible()) {
                // 在路口，更新意图
                // 更新检查点
                checkPoint_ = cor;
                // chase模式
                if (Ghost::Mode::Chase == mode) {
                    if (aiMap_.find(name) == aiMap_.end()) {
                        intentionDir = aiBlinky_(pacman, *this);
                    } else {
                        intentionDir = aiMap_[name](pacman, *this);
                    }
                } else if (Ghost::Mode::Scatter == mode) {
                    // Scatter模式
                    if (!scatterInfo_.scatterCheckPoint &&
                        cor.x == scatterInfo_.scatterPoint.x &&
                        cor.y == scatterInfo_.scatterPoint.y) {
                        scatterInfo_.scatterCheckPoint = true;
                        scatterInfo_.scatterCCW = leftTile.IsGhostAccessible();
                    }
                    if (scatterInfo_.scatterCheckPoint) {
                        intentionDir = scatterInfo_.scatterCCW ? leftD : rightD;
                    } else {
                        intentionDir = aiMap_["Frightened"](pacman, *this);
                    }
                } else if (Ghost::Mode::Frightened == mode) {
                    // Frightened模式
                    // 1/7 保持直行(可能会碰壁、掉头)，6/7拐弯
                    int randNum = std::rand() % 7;
                    if (randNum < 3) {
                        if (leftTile.IsGhostAccessible()) {
                            intentionDir = leftD;
                        } else {
                            intentionDir = rightD;
                        }
                    } else if (randNum < 6) {
                        if (rightTile.IsGhostAccessible()) {
                            intentionDir = rightD;
                        } else {
                            intentionDir = leftD;
                        }
                    }
                }
            }
        }
    }

    Monster::Update();
}

void Ghost::Debug() {
    std::unique_ptr<SDL_Point[]> path_(new SDL_Point[path.size()]);
    for (int i = 0; i < path.size(); i++) {
        int randX = std::rand() % 3 - 1;
        int randY = std::rand() % 3 - 1;
        path_[i] = {
            static_cast<int>(path[i].x * TileSize + TileSize / 2) + randX,
            static_cast<int>(path[i].y * TileSize + TileSize / 2) + randY};
    }
    Context::GetInstance().GetRenderer().DrawPath(path_, color_, path.size());
}

void Ghost::InitAiMap() {
    aiMap_.emplace("Blinky", aiBlinky_);
    aiMap_.emplace("Pinky", aiPinky_);
    aiMap_.emplace("Inky", aiInky_);
    aiMap_.emplace("Clyde", aiClyde_);
    aiMap_.emplace("Frightened", aiScatter_);
    aiMap_.emplace("Waiting", aiWaiting_);
}

void Ghost::ChangeMode(Mode _mode) {
    mode = _mode;
    // 直接重置
    scatterInfo_.scatterCCW = false;
    scatterInfo_.scatterCheckPoint = false;
    if (mode == Mode::Frightened) {
        frightenedTime = FrightenedTime;
        speed = 2.0f;
        GetImage().color = FrightenedColor;
    } else {
        frightenedTime = 0.0f;
        speed = 5.0f;
        GetImage().color = getColor(name);
    }
}

Monster::Direction GetDirectionFromPath(
    const std::vector<MapCoordinate>& path) {
    if (path.size() < 2) {
        return Monster::Direction::Up;
    }
    int offsetX = path[0].x - path[1].x;
    int offsetY = path[0].y - path[1].y;
    if (offsetX == -1) {
        return Monster::Direction::Right;
    }
    if (offsetX == 1) {
        return Monster::Direction::Left;
    }
    if (offsetY == -1) {
        return Monster::Direction::Down;
    }
    if (offsetY == 1) {
        return Monster::Direction::Up;
    }
    return Monster::Direction::Up;
}

Ghost::AIType Ghost::aiBlinky_ = [](Pacman& pacman, Ghost& ghost) {
    auto& gameCtx = GameContext::GetInstance();
    ghost.path = gameCtx.gameMap->ShortestPathBetweenTiles(
        pacman.GetMapCorrdinate(), ghost.GetMapCorrdinate());

    return GetDirectionFromPath(ghost.path);
};

Ghost::AIType Ghost::aiPinky_ = [](Pacman& pacman, Ghost& ghost) {
    auto& gameCtx = GameContext::GetInstance();
    auto sourceCor = pacman.GetMapCorrdinate();
    // 2 pac front of pacman
    auto pacMoveCor = DirectionToCoordinate(pacman.movingDir);
    auto intentionCor = DirectionToCoordinate(pacman.intentionDir);
    int pacCount = 2;
    std::queue<BFSNode> queue;
    queue.push({sourceCor.x, sourceCor.y, 0, 0});
    while (!queue.empty()) {
        auto& now = queue.front();
        queue.pop();
        // 防止只能走到0或1步
        sourceCor = {now.x, now.y};
        if (now.step == pacCount) {
            // 走到第pacCount步，直接结束
            break;
        }
        if (gameCtx.gameMap->GetTile(now.x + pacMoveCor.x, now.y + pacMoveCor.y)
                .IsGhostAccessible()) {
            queue.push(
                {now.x + pacMoveCor.x, now.y + pacMoveCor.y, now.step + 1, 0});
        }
        if (gameCtx.gameMap
                ->GetTile(now.x + intentionCor.x, now.y + intentionCor.y)
                .IsGhostAccessible()) {
            queue.push({now.x + intentionCor.x, now.y + intentionCor.y,
                        now.step + 1, 0});
        }
    }

    ghost.path = gameCtx.gameMap->ShortestPathBetweenTiles(
        sourceCor, ghost.GetMapCorrdinate());

    return GetDirectionFromPath(ghost.path);
};

Ghost::AIType Ghost::aiInky_ = [](Pacman& pacman, Ghost& ghost) {
    auto& gameCtx = GameContext::GetInstance();
    auto pinkyTargetCor = pacman.GetMapCorrdinate();
    // 2 pac front of pacman
    auto pacMoveCor = DirectionToCoordinate(pacman.movingDir);
    auto intentionCor = DirectionToCoordinate(pacman.intentionDir);
    int pacCount = 2;
    std::queue<BFSNode> queue;
    queue.push({pinkyTargetCor.x, pinkyTargetCor.y, 0, 0});
    while (!queue.empty()) {
        auto& now = queue.front();
        queue.pop();
        // 防止只能走到0或1步
        pinkyTargetCor = {now.x, now.y};
        if (now.step == pacCount) {
            // 走到第pacCount步，直接结束
            break;
        }
        if (gameCtx.gameMap->GetTile(now.x + pacMoveCor.x, now.y + pacMoveCor.y)
                .IsGhostAccessible()) {
            queue.push(
                {now.x + pacMoveCor.x, now.y + pacMoveCor.y, now.step + 1, 0});
        }
        if (gameCtx.gameMap
                ->GetTile(now.x + intentionCor.x, now.y + intentionCor.y)
                .IsGhostAccessible()) {
            queue.push({now.x + intentionCor.x, now.y + intentionCor.y,
                        now.step + 1, 0});
        }
    }

    auto inkyCor = ghost.GetMapCorrdinate();
    auto sourceCor = pinkyTargetCor * 2 - inkyCor;
    sourceCor = gameCtx.gameMap->NearestAccessibleTile(sourceCor);
    ghost.path = gameCtx.gameMap->ShortestPathBetweenTiles(sourceCor, inkyCor);

    return GetDirectionFromPath(ghost.path);
};

Ghost::AIType Ghost::aiClyde_ = [](Pacman& pacman, Ghost& ghost) {
    auto& gameCtx = GameContext::GetInstance();
    ghost.path = gameCtx.gameMap->ShortestPathBetweenTiles(
        pacman.GetMapCorrdinate(), ghost.GetMapCorrdinate());
    if (ghost.path.size() > 8) {
        return GetDirectionFromPath(ghost.path);
    } else {
        ghost.path = gameCtx.gameMap->ShortestPathBetweenTiles(
            ghost.scatterInfo_.scatterPoint, ghost.GetMapCorrdinate());
        return GetDirectionFromPath(ghost.path);
    }
};

Ghost::AIType Ghost::aiScatter_ = [](Pacman& pacman, Ghost& ghost) {
    auto& gameCtx = GameContext::GetInstance();
    ghost.path = gameCtx.gameMap->ShortestPathBetweenTiles(
        ghost.scatterInfo_.scatterPoint, ghost.GetMapCorrdinate());
    return GetDirectionFromPath(ghost.path);
};

Ghost::AIType Ghost::aiWaiting_ = [](Pacman& pacman, Ghost& ghost) {
    auto& gameCtx = GameContext::GetInstance();
    if (std::fmod(gameCtx.globalElapsed, 1.0f) < 0.5) {
        return Direction::Down;
    } else {
        return Direction::Up;
    }
};
