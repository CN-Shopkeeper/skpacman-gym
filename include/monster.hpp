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

    Image image;
    float speed = 5.0f;

    Monster(const Image& image, const Vector2& position)
        : image(image), position_(position) {}

    virtual ~Monster() = default;

    static MapCoordinate DirectionToCoordinate(Direction& direction);

    void Move(const Vector2& offset) {
        offset_ = offset;
        position_ += offset;
    }

    auto& GetPosition() const { return position_; }
    auto& GetVelocity() const { return offset_; }

    virtual void Reset(const Vector2& position) {
        position_ = position;
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
    virtual void Debug() = 0;

   protected:
    Vector2 offset_;
    Vector2 position_;
    bool isTurnBack() const;
    void doUpdate();
};

class Pacman : public Monster {
   public:
    Pacman(const Image& image, const Vector2& position)
        : Monster(image, position) {}

    void Debug() override{};

   private:
};

class Ghost : public Monster {
   public:
    enum Mode { Chase = 0, Scatter = 1, Frightened = 2 };
    Ghost(const Image& _image, const Vector2& position, std::string name,
          SDL_Color color, MapCoordinate scatterPoint)
        : Monster(_image, position), name(name), color_(color) {
        speed = 3;
        checkPoint_ = {-1, -1};
        mode = Mode::Chase;
        image.SetColorMod(color);
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

    void Reset(const Vector2& position) override {
        Monster::Reset(position);
        if (name == "Blinky" || name == "Pinky") {
            joinChasing = true;
        } else {
            joinChasing = false;
        }
    }

    static void InitAiMap();
    void Update() override;
    void Debug() override;
    void ChangeMode(Mode mode);
    bool IsFrightened() const { return mode == Mode::Frightened; }

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
    inline static std::unordered_map<std::string, AIType> aiMap_ =
        std::unordered_map<std::string, AIType>();
    static AIType aiPinky_;
    static AIType aiBlinky_;
    static AIType aiInky_;
    static AIType aiClyde_;
    static AIType aiScatter_;
    static AIType aiWaiting_;
};

Monster::Direction GetDirectionFromPath(const std::vector<MapCoordinate>& path);
