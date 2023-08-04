#pragma once

#include "consts.hpp"
#include "pch.hpp"

struct Tile final {
    enum class Type {
        Empty = -1,
        Wall = static_cast<int>(ImageTileType::Wall),
        Bean = static_cast<int>(ImageTileType::Bean),
        PowerBean = static_cast<int>(ImageTileType::PowerBean),
        GhostDoor = static_cast<int>(ImageTileType::GhostDoor)
    };
    Type type = Type::Empty;
};

class Map final {
   public:
    Map(std::string_view desc, const Size& size);

    const Tile& GetTile(int x, int y) const { return tiles_->Get(x, y); }
    int Width() const { return tiles_->Width(); }
    int Height() const { return tiles_->Height(); }

    void Draw();

   private:
    std::unique_ptr<Matrix<Tile>> tiles_;
};
