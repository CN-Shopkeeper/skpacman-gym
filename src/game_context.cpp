#include "game_context.hpp"

#include "context.hpp"

GameContext::GameContext() {
    gameMap.reset(new Map(MapDesc, {MapWidth, MapHeight}));

    auto& ctx = Context::GetInstance();
    auto tilesheet = ctx.GetTextureManager().FindTilesheet(TilesheetName);
    
}