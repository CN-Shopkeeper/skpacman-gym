#pragma once

#include "consts.hpp"
#include "context.hpp"
#include "map.hpp"
#include "pch.hpp"
#include "texture.hpp"

class Monster {
   public:
    enum class Direction { Right = 0, Down, Left, Up };

    Direction movingDir = Direction::Right;
    Direction intentionDir = Direction::Right;

    std::vector<Image> images;
    float speed = 5.0f;

    Monster(const std::vector<Image>&& _images) : images(std::move(_images)) {}

    virtual ~Monster() = default;

    static MapCoordinate DirectionToCoordinate(Direction& direction);

    void Move(const Vector2& offset) {
        offset_ = offset;
        position_ += offset;
    }

    auto& GetPosition() const { return position_; }
    auto& GetVelocity() const { return offset_; }

    virtual void Reset() {
        position_ = getInitPosition();
        movingDir = Direction::Right;
        intentionDir = Direction::Right;
    }

    Rect GetRect() const { return Rect(position_, {TileSize, TileSize}); }

    MapCoordinate GetMapCorrdinate() const {
        Rect rect = GetRect();
        auto center = rect.Center();
        return {static_cast<int>(center.x / TileSize),
                static_cast<int>(center.y / TileSize)};
    }

    Direction LeftDirection() const {
        return static_cast<Direction>((static_cast<int>(movingDir) - 1 + 4) %
                                      4);
    }

    Direction RightDirection() const {
        return static_cast<Direction>((static_cast<int>(movingDir) + 1 + 4) %
                                      4);
    }

    Direction BackDirection() const {
        return static_cast<Direction>((static_cast<int>(intentionDir) + 2) % 4);
    }

    /**
     * @brief  判断怪物是否抵达某一个Tile
     * @note
     * 实际使用中并不是point in
     * rect就能判断角色等是否已经处于Tile中，往往需要当角色到达Tile的中心才可以
     * 这个函数两个参数还是比较抽象的，具体的判定范围为：
     * 在怪物前进方向上的
     * `monster.speed ×(-threshold+ offset)`
     * 到`monster.speed × (threshold + offset)`之间
     * @param  targetTile: 目标Tile坐标
     * @param  threshold: 判定阈值，应当大于等于0.5
     * @param  offset: 阈值偏移量
     * @retval 是否抵达Tile
     */
    bool ReachTheTile(float threshold, float offset);

    void Draw();
    virtual void Update();
    virtual Image& GetImage() = 0;
    virtual void Debug() = 0;

   protected:
    Vector2 offset_;
    Vector2 position_;
    bool isTurnBack() const;
    void doUpdate();
    virtual Vector2 getInitPosition() = 0;
    virtual bool isAccessible(Map& gameMap,
                              const MapCoordinate& coor) const = 0;
};

class Pacman : public Monster {
   public:
    Pacman(const std::vector<Image>&& _images) : Monster(std::move(_images)) {
        position_ = getInitPosition();
    }

    void Debug() override{};
    Image& GetImage() override;
    Vector2 getInitPosition() override {
        return Vector2{PacmanInitX, PacmanInitY};
    }

   private:
    virtual bool isAccessible(Map& gameMap,
                              const MapCoordinate& coor) const override {
        return gameMap.GetTile(coor).IsPacmanAccessible();
    };
};

class Ghost : public Monster {
   public:
    enum Mode { Chase = 0, Scatter = 1, Frightened = 2 };
    Ghost(const std::vector<Image>&& _images, std::string _name,
          MapCoordinate scatterPoint)
        : Monster(std::move(_images)), name(_name), color_(getColor(_name)) {
        speed = 3;
        checkPoint_ = {-1, -1};
        mode = Mode::Chase;
        position_ = getInitPosition();
        for (auto& image : images) {
            image.SetColorMod(color_);
        }
        scatterInfo_.scatterPoint = scatterPoint;
        if (name == "Blinky" || name == "Pinky") {
            joinChasing = true;
        } else {
            joinChasing = false;
        }
    }
    std::string name;
    Mode mode;
    bool joinChasing = false;
    std::vector<MapCoordinate> path;

    void Reset() override {
        Monster::Reset();
        if (name == "Blinky" || name == "Pinky") {
            joinChasing = true;
        } else {
            joinChasing = false;
        }
    }

    static void InitAiMap();
    void Update() override;
    Image& GetImage() override { return images[0]; }
    void Debug() override;
    void ChangeMode(Mode mode);
    bool IsFrightened() const { return mode == Mode::Frightened; }
    SDL_Color GetColor() const { return getColor(name); }
    MapCoordinate GetScatterPoint() const { return scatterInfo_.scatterPoint; }

    std::string GetModeStr() const {
        switch (mode) {
            case Mode::Chase:
                return "Chase";
            case Mode::Scatter:
                return "Scatter";
            case Mode::Frightened:
                return "Frightened";
            default:
                return "Chase";
        }
    }

    using AIType = std::function<Direction(Pacman&, Ghost&)>;

   private:
    struct ScatterInfo {
        // Scatter模式下的点
        MapCoordinate scatterPoint;
        // Scatter模式下的检查点，是否到达scatterPoint_，如果到达则开始绕圈
        bool scatterCheckPoint = false;
        // Scatter模式下，到达scaterPoint_之后，绕圈的转向
        bool scatterCCW = false;
    };
    SDL_Color color_;
    // 上次转向判定点，按照wiki说明只有在路口时才能转向判定。
    MapCoordinate checkPoint_;
    ScatterInfo scatterInfo_;
    Vector2 getInitPosition() override { return getInitPositionByName(name); }
    virtual bool isAccessible(Map& gameMap,
                              const MapCoordinate& coor) const override {
        return gameMap.GetTile(coor).IsGhostAccessible();
    };
    inline static std::unordered_map<std::string, AIType> aiMap_ =
        std::unordered_map<std::string, AIType>();
    static AIType aiPinky_;
    static AIType aiBlinky_;
    static AIType aiInky_;
    static AIType aiClyde_;
    static AIType aiScatter_;
    static AIType aiWaiting_;
    static SDL_Color getColor(const std::string& name) {
        if ("Blinky" == name) {
            return BlinkyColor;
        }
        if ("Pinky" == name) {
            return PinkyColor;
        }
        if ("Inky" == name) {
            return InkyColor;
        }
        if ("Clyde" == name) {
            return ClydeColor;
        }
        return BlinkyColor;
    }

    static Vector2 getInitPositionByName(const std::string& name) {
        if ("Blinky" == name) {
            return Vector2{GhostInitX + TileSize * 2, GhostInitY};
        }
        if ("Pinky" == name) {
            return Vector2{GhostInitX + TileSize, GhostInitY};
        }
        if ("Inky" == name) {
            return Vector2{GhostInitX, GhostInitY};
        }
        if ("Clyde" == name) {
            return Vector2{GhostInitX + TileSize * 3, GhostInitY};
        }
        return Vector2{GhostInitX + TileSize * 2, GhostInitY};
    }
};

Monster::Direction GetDirectionFromPath(const std::vector<MapCoordinate>& path);

inline std::ostream& operator<<(std::ostream& stream, const Ghost::Mode& mode) {
    switch (mode) {
        case Ghost::Mode::Chase:
            stream << "Chase";
            break;
        case Ghost::Mode::Scatter:
            stream << "Scatter";
            break;
        case Ghost::Mode::Frightened:
            stream << "Frightened";
            break;
    }
    return stream;
}
