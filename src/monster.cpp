#include "monster.hpp"

#include "game_context.hpp"

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
}

void Monster::Update() {
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
void Pacman::Update() {
    if (intentionDir == movingDir) {
        Monster::Update();
    } else if (isTurnBack()) {
        movingDir = intentionDir;
        Monster::Update();
    } else {
        // todo: remove this magic number
        Rect monsterRect(GetPosition(), {32, 32});
        auto monsterCenter = monsterRect.Center();
        int indexX = static_cast<int>(monsterCenter.x / 32);
        int indexY = static_cast<int>(monsterCenter.y / 32);
        Rect tile = Rect{Vector2{indexX * 32.f, indexY * 32.f}, {32, 32}};
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
            Monster::Update();
        } else {
            Move(diff);
            movingDir = intentionDir;
        }
    }
}

bool Pacman::isTurnBack() const {
    return (4 + static_cast<int>(intentionDir) - static_cast<int>(movingDir)) %
               4 ==
           2;
}
