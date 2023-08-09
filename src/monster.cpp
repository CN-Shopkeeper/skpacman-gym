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
    if (cor.x == checkPoint_.x && cor.y == checkPoint_.y) {
        // 刚过检查点，直接进行更新
        Monster::Update();
    } else {
        // 检查是否在路口
        auto leftD = LeftDirection();
        auto rightD = RightDirection();
        auto left = DirectionToCoordinate(leftD) + cor;
        auto right = DirectionToCoordinate(rightD) + cor;
        if (gameCtx.gameMap->GetTile(left.x, left.y).IsAccessible() ||
            gameCtx.gameMap->GetTile(right.x, right.y).IsAccessible()) {
            // 在路口，更新意图
            auto& pacman = gameCtx.controller->pacman;
            if (aiMap_.find(name_) == aiMap_.end()) {
                intentionDir = aiBlinky_(pacman, *this, path);
            } else {
                intentionDir = aiMap_[name_](pacman, *this, path);
            }
            // 更新检查点
            checkPoint_ = cor;
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
        path_[i] = {static_cast<int>(path[i].x * TileSize + TileSize / 2),
                    static_cast<int>(path[i].y * TileSize + TileSize / 2)};
    }
    Context::GetInstance().GetRenderer().DrawPath(path_, color_, path.size());
}

void Ghost::InitAiMap() {
    aiMap_.emplace("Blinky", aiBlinky_);
    aiMap_.emplace("Pinky", aiPinky_);
    aiMap_.emplace("Inky", aiInky_);
    aiMap_.emplace("Clyde", aiClyde_);
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

Ghost::AIType Ghost::aiBlinky_ = [](Pacman& pacman, Ghost& ghost,
                                    std::vector<MapCoordinate>& path) {
    auto& gameCtx = GameContext::GetInstance();
    path = gameCtx.gameMap->ShortestPathBetweenTiles(pacman.GetMapCorrdinate(),
                                                     ghost.GetMapCorrdinate());
    return GetDirectionFromPath(path);
};

Ghost::AIType Ghost::aiPinky_ = [](Pacman& pacman, Ghost& ghost,
                                   std::vector<MapCoordinate>& path) {
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

    path = gameCtx.gameMap->ShortestPathBetweenTiles(sourceCor,
                                                     ghost.GetMapCorrdinate());

    return GetDirectionFromPath(path);
};

Ghost::AIType Ghost::aiInky_ = [](Pacman& pacman, Ghost& ghost,
                                  std::vector<MapCoordinate>& path) {
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
    path = gameCtx.gameMap->ShortestPathBetweenTiles(sourceCor, inkyCor);

    return GetDirectionFromPath(path);
};

Ghost::AIType Ghost::aiClyde_ = [](Pacman& pacman, Ghost& ghost,
                                   std::vector<MapCoordinate>& path) {
    auto& gameCtx = GameContext::GetInstance();
    path = gameCtx.gameMap->ShortestPathBetweenTiles(pacman.GetMapCorrdinate(),
                                                     ghost.GetMapCorrdinate());
    if (path.size() > 8) {
        return GetDirectionFromPath(path);
    } else {
        path = gameCtx.gameMap->ShortestPathBetweenTiles(
            ghost.scatterPoint_, ghost.GetMapCorrdinate());
        return GetDirectionFromPath(path);
    }
};
;
