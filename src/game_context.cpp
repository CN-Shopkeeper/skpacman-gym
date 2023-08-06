#include "game_context.hpp"

#include "context.hpp"

GameContext::GameContext() {
    gameMap.reset(new Map(Map::GenerateMap(), {MapWidth, MapHeight}));

    auto& ctx = Context::GetInstance();
    auto tilesheet = ctx.GetTextureManager().FindTilesheet(TilesheetName);

    monsters.emplace_back(
        new Pacman{tilesheet->Get(static_cast<int>(ImageTileType::Pacman), 0),
                   Vector2{48, 32}});
    controller.reset(new Controller(*dynamic_cast<Pacman*>(monsters[0].get())));
}

void GameContext::dealCollideWithMap(Monster& monster) {
    // todo: remove this magic number
    Rect monsterRect(monster.GetPosition(), {32, 32});
    auto monsterCenter = monsterRect.Center();
    int cx = monsterCenter.x / 32;
    int cy = monsterCenter.y / 32;
    for (int dx = -1; dx <= 1; dx++) {
        for (int dy = -1; dy <= 1; dy++) {
            auto x = dx + cx, y = dy + cy;
            if (x < 0 || x > MapWidth || y < 0 || y > MapHeight) {
                continue;
            }
            Rect tileRect(Vector2(x * 32, y * 32), {32, 32});
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
                monsterRect = Rect(monster.GetPosition(), {32, 32});
            }
        }
    }
}

void GameContext::Update() {
    for (auto& monster : monsters) {
        monster->Update();
    }

    for (auto& monster : monsters) {
        dealCollideWithMap(*monster);
    }
}
