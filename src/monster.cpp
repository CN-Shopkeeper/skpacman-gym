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
    renderer.DrawImage(image, position_, scale * Scale, rotation);

    if (GameContext::GetInstance().debugMode) {
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

// 可以在转向时对位置进行一定的容差
// 转向时，将转向前的方向对齐到网格上（阈值为速度的-20%~100%）
void Monster::Update() {
    if (intentionDir == movingDir) {
        Monster::doUpdate();
    } else if (isTurnBack()) {
        movingDir = intentionDir;
        Monster::doUpdate();
    } else {
        Rect monsterRect(GetPosition(), {TileSize, TileSize});
        auto monsterCenter = monsterRect.Center();
        int indexX = static_cast<int>(monsterCenter.x / TileSize);
        int indexY = static_cast<int>(monsterCenter.y / TileSize);
        Rect tile =
            Rect{Vector2{indexX * TileSize * 1.f, indexY * TileSize * 1.f},
                 {TileSize, TileSize}};
        auto tileCenter = tile.Center();
        auto diff = tileCenter - monsterCenter;
        bool should = false;

        auto& gameCtx = GameContext::GetInstance();

        // constM 用于计算阈值范围
        double constM = 1.5 - static_cast<int>(movingDir) > 0 ? -0.4 : 0.4;
        int constI = 1.5 - static_cast<int>(intentionDir) > 0 ? 1 : -1;
        if (movingDir == Direction::Left || movingDir == Direction::Right) {
            if (diff.x >= speed * (constM - 0.6) &&
                diff.x <= speed * (constM + 0.6)) {
                // todo check boundaries
                should = gameCtx.gameMap
                             ->GetTile(static_cast<int>(indexX),
                                       static_cast<int>(indexY + constI))
                             .type != Tile::Type::Wall;
            }
        }
        if (movingDir == Direction::Up || movingDir == Direction::Down) {
            if (diff.y >= speed * (constM - 0.6) &&
                diff.y <= speed * (constM + 0.6)) {  // todo check boundaries
                should = gameCtx.gameMap
                             ->GetTile(static_cast<int>(indexX + constI),
                                       static_cast<int>(indexY))
                             .type != Tile::Type::Wall;
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

void Ghost::Update() {
    auto& gameCtx = GameContext::GetInstance();
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
                 .IsAccessible()) {
            auto forward = DirectionToCoordinate(movingDir) + cor;
            if (gameCtx.gameMap->GetTile(forward.x, forward.y).IsAccessible()) {
                intentionDir = movingDir;
            } else if (gameCtx.gameMap->GetTile(left.x, left.y)
                           .IsAccessible()) {
                intentionDir = leftD;
            } else if (gameCtx.gameMap->GetTile(right.x, right.y)
                           .IsAccessible()) {
                intentionDir = rightD;
            } else {
                intentionDir = BackDirection();
            }
        }
        // if (intentionDir == Direction::Up) {
        //     if (!gameCtx.gameMap->GetTile(cor.x, cor.y - 1).IsAccessible()) {
        //         intentionDir = Direction::Down;
        //     }
        // }
        // 直接进行更新
        Monster::Update();
    } else {
        // 检查是否在路口
        auto& leftTile = gameCtx.gameMap->GetTile(left.x, left.y);
        auto& rightTile = gameCtx.gameMap->GetTile(right.x, right.y);
        if (leftTile.IsAccessible() || rightTile.IsAccessible()) {
            // 在路口，更新意图
            // 更新检查点
            checkPoint_ = cor;
            auto& pacman = gameCtx.controller->pacman;
            // chase模式
            if (Ghost::Mode::Chase == mode_) {
                if (aiMap_.find(name_) == aiMap_.end()) {
                    intentionDir = aiBlinky_(pacman, *this);
                } else {
                    intentionDir = aiMap_[name_](pacman, *this);
                }
            } else if (Ghost::Mode::Scatter == mode_) {
                // Scatter模式
                if (!scatterInfo_.scatterCheckPoint &&
                    cor.x == scatterInfo_.scatterPoint.x &&
                    cor.y == scatterInfo_.scatterPoint.y) {
                    scatterInfo_.scatterCheckPoint = true;
                    scatterInfo_.scatterCCW = leftTile.IsAccessible();
                }
                if (scatterInfo_.scatterCheckPoint) {
                    intentionDir = scatterInfo_.scatterCCW ? leftD : rightD;
                } else {
                    intentionDir = aiMap_["Frightened"](pacman, *this);
                }
            } else if (Ghost::Mode::Frightened == mode_) {
                // Frightened模式
                // 1/7 保持直行(可能会碰壁、掉头)，6/7拐弯
                int randNum = std::rand() % 7;
                if (randNum < 3) {
                    if (leftTile.IsAccessible()) {
                        intentionDir = leftD;
                    } else {
                        intentionDir = rightD;
                    }
                } else if (randNum < 6) {
                    if (rightTile.IsAccessible()) {
                        intentionDir = rightD;
                    } else {
                        intentionDir = leftD;
                    }
                }
            }
        }
        Monster::Update();
    }

    // intentionDir = aiBlinky_(pacman, *this);
    // std::cout << static_cast<int>(movingDir) << std::endl;
    // movingDir = aiMap_[name_](pacman, *this);
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
    aiMap_.emplace("Frightened", aiFrightened_);
}

void Ghost::ChangeMode(Mode mode) {
    mode_ = mode;
    // 直接重置
    scatterInfo_.scatterCCW = false;
    scatterInfo_.scatterCheckPoint = false;
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
                .IsAccessible()) {
            queue.push(
                {now.x + pacMoveCor.x, now.y + pacMoveCor.y, now.step + 1, 0});
        }
        if (gameCtx.gameMap
                ->GetTile(now.x + intentionCor.x, now.y + intentionCor.y)
                .IsAccessible()) {
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
                .IsAccessible()) {
            queue.push(
                {now.x + pacMoveCor.x, now.y + pacMoveCor.y, now.step + 1, 0});
        }
        if (gameCtx.gameMap
                ->GetTile(now.x + intentionCor.x, now.y + intentionCor.y)
                .IsAccessible()) {
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

Ghost::AIType Ghost::aiFrightened_ = [](Pacman& pacman, Ghost& ghost) {
    auto& gameCtx = GameContext::GetInstance();
    ghost.path = gameCtx.gameMap->ShortestPathBetweenTiles(
        ghost.scatterInfo_.scatterPoint, ghost.GetMapCorrdinate());
    return GetDirectionFromPath(ghost.path);
};
