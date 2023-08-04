#include "map.hpp"

#include "context.hpp"

Map::Map(std::string_view desc, const Size& size) {
    tiles_.reset(
        new Matrix<Tile>(static_cast<int>(size.w), static_cast<int>(size.h)));
    for (int y = 0; y < size.h; y++) {
        for (int x = 0; x < size.x; x++) {
            char c = desc[x + y * size.w];
            Tile::Type type;
            switch (c) {
                case 'W':
                    type = Tile::Type::Wall;
                    break;
                case 'B':
                    type = Tile::Type::Bean;
                    break;
                case 'P':
                    type = Tile::Type::PowerBean;
                    break;
                case ' ':
                    type = Tile::Type::Empty;
                    break;
                case 'G':
                    type = Tile::Type::GhostDoor;
                    break;
                default:
                    break;
            }
            tiles_->Get(x, y).type = type;
        }
    }
}

void Map::Draw() {
    auto& ctx = Context::GetInstance();
    auto& renderer = ctx.GetRenderer();
    auto tilesheet = ctx.GetTextureManager().FindTilesheet(TilesheetName);
    for (int y = 0; y < tiles_->Height(); y++) {
        for (int x = 0; x < tiles_->Width(); x++) {
            auto tileType = tiles_->Get(x, y).type;
            renderer.DrawImage(tilesheet->Get(static_cast<int>(tileType), 0),
                               {x * tilesheet->GetTileSize().w * Scale,
                                y * tilesheet->GetTileSize().h * Scale},
                               {Scale, Scale});
        }
    }
}
