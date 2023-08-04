#include "game_context.hpp"

#include "context.hpp"

GameContext::GameContext() {
    gameMap.reset(new Map(MapDesc, {MapWidth, MapHeight}));

    auto& ctx = Context::GetInstance();
    auto tilesheet = ctx.GetTextureManager().FindTilesheet(TilesheetName);
    monsters.emplace_back(
        tilesheet->Get(static_cast<int>(ImageTileType::Pacman), 0),
        Vector2{48, 32});
}

void GameContext::Update() {
    for (auto& monster : monsters) {
        monster.Update();
    }
}
