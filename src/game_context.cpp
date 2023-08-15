#include "game_context.hpp"

#include "context.hpp"

GameContext::GameContext() {
    gameMap.reset(new Map(Map::GenerateMap(beanCount_), {MapWidth, MapHeight}));

    auto& ctx = Context::GetInstance();
    auto tilesheet = ctx.GetTextureManager().FindTilesheet(TilesheetName);

    Ghost::InitAiMap();

    monsters.emplace_back(
        new Pacman{tilesheet->Get(static_cast<int>(ImageTileType::Pacman), 0),
                   Vector2{PacmanInitX, PacmanInitY}});
    monsters.emplace_back(
        new Ghost{tilesheet->Get(static_cast<int>(ImageTileType::Ghost), 0),
                  Vector2{GhostInitX, GhostInitY}, "Blinky", BlinkyColor,
                  gameMap->NearestAccessibleTile({MapWidth, -1})});
    monsters.emplace_back(
        new Ghost{tilesheet->Get(static_cast<int>(ImageTileType::Ghost), 0),
                  Vector2{GhostInitX + TileSize, GhostInitY}, "Pinky",
                  PinkyColor, gameMap->NearestAccessibleTile({-1, -1})});
    monsters.emplace_back(new Ghost{
        tilesheet->Get(static_cast<int>(ImageTileType::Ghost), 0),
        Vector2{GhostInitX + TileSize * 2, GhostInitY}, "Inky", InkyColor,
        gameMap->NearestAccessibleTile({MapWidth, MapHeight})});
    monsters.emplace_back(
        new Ghost{tilesheet->Get(static_cast<int>(ImageTileType::Ghost), 0),
                  Vector2{GhostInitX + TileSize * 2, GhostInitY}, "Clyde",
                  ClydeColor, gameMap->NearestAccessibleTile({-1, MapHeight})});
    controller.reset(new Controller(*dynamic_cast<Pacman*>(monsters[0].get())));
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
    for (auto& monster : monsters) {
        monster->Update();
    }

    for (auto& monster : monsters) {
        dealCollideWithMap(*monster);
    }
    tryEatBean();
}

void GameContext::newGame() {
    gameMap.reset(new Map(Map::GenerateMap(beanCount_), {MapWidth, MapHeight}));

    monsters[0]->Reset(Vector2{PacmanInitX, PacmanInitY});
    monsters[1]->Reset(Vector2{GhostInitX, GhostInitY});
    monsters[2]->Reset(Vector2{GhostInitX + TileSize, GhostInitY});
    monsters[3]->Reset(Vector2{GhostInitX + TileSize * 2, GhostInitY});
    monsters[4]->Reset(Vector2{GhostInitX + TileSize * 3, GhostInitY});
}

void GameContext::tryEatBean() {
    auto& pacman = controller->pacman;
    auto pacmanCor = pacman.GetMapCorrdinate();
    bool reach = pacman.ReachTheTile(0.6, 0);
    Tile& tile = gameMap->GetTile(pacmanCor);
    if (reach && tile.type == Tile::Type::Bean) {
        tile.type = Tile::Type::Empty;
        score_++;
        updateScoreText();
    }
}
