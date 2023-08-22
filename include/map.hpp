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
    bool IsGhostAccessible() const {
        return type != Type::Wall;
        // return type != Type::Wall && type != Type::GhostDoor;
    }
    bool IsPacmanAccessible() const {
        return type != Type::Wall && type != Type::GhostDoor;
    }
};

class Tetris final {
   public:
    template <const size_t height, const size_t width>
    static std::array<int, height * width> GenerateTetris();
    inline static int randSeed = 0;

   private:
    template <const size_t height, const size_t width>
    static void bfs(std::array<int, height * width>& terris, int x, int y,
                    int cnt, int num);

    static int getTreisCount();
};

class MapCoordinate {
   public:
    int x;
    int y;

    MapCoordinate operator+(const MapCoordinate& other) const {
        return {x + other.x, y + other.y};
    }

    MapCoordinate operator-(const MapCoordinate& other) const {
        return {x - other.x, y - other.y};
    }

    MapCoordinate operator*(const int value) const {
        return {x * value, y * value};
    }

    bool operator==(const MapCoordinate& other) const {
        return x == other.x && y == other.y;
    }
};

inline MapCoordinate operator*(int value, const MapCoordinate& v) {
    return v * value;
}

struct BFSNode {
    int x;
    int y;
    int step;
    int pre;
};

class Map final {
   public:
    Map(std::string_view desc, const Size& size);
    static std::string GenerateMap(int& beanCount);

    const Tile& GetTile(int x, int y) const { return tiles_->Get(x, y); }
    const Tile& GetTile(MapCoordinate cor) const {
        return tiles_->Get(cor.x, cor.y);
    }
    Tile& GetTile(MapCoordinate cor) { return tiles_->Get(cor.x, cor.y); }
    int Width() const { return tiles_->Width(); }
    int Height() const { return tiles_->Height(); }

    void Draw();

    // 返回从终点到起点的路径
    std::vector<MapCoordinate> ShortestPathBetweenTiles(MapCoordinate source,
                                                        MapCoordinate target);

    // 找寻最近可到达的tile
    MapCoordinate NearestAccessibleTile(MapCoordinate target);

    // 坐标是否在地图内
    bool IsInside(int x, int y) const {
        return x >= 0 && x < Width() && y >= 0 && y < Height();
    }

    bool IsInside(MapCoordinate cor) const { return IsInside(cor.x, cor.y); }

   private:
    std::unique_ptr<Matrix<Tile>> tiles_;
};
