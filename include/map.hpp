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
    bool IsAccessible() const {
        return type != Type::Wall;
        // return type != Type::Wall && type != Type::GhostDoor;
    }
};

class Tetris final {
   public:
    template <const size_t height, const size_t width>
    static std::array<int, height * width> GenerateTetris();

   private:
    template <const size_t height, const size_t width>
    static void bfs(std::array<int, height * width>& terris, int x, int y,
                    int cnt, int num);

    static int getTreisCount();
};

struct MapCoordinate {
    int x;
    int y;

    MapCoordinate operator+(const MapCoordinate& other) {
        return {x + other.x, y + other.y};
    }
};

struct BFSNode {
    int x;
    int y;
    int step;
    int pre;
};

class Map final {
   public:
    Map(std::string_view desc, const Size& size);
    static std::string GenerateMap();

    const Tile& GetTile(int x, int y) const { return tiles_->Get(x, y); }
    int Width() const { return tiles_->Width(); }
    int Height() const { return tiles_->Height(); }

    void Draw();

    // 返回从终点到起点的路径
    std::vector<MapCoordinate> ShortestPathBetweenTiles(MapCoordinate source,
                                                        MapCoordinate target);

   private:
    std::unique_ptr<Matrix<Tile>> tiles_;
};
