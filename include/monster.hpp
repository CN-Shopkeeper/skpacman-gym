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

    void Move(const Vector2& offset) {
        offset_ = offset;
        position_ += offset;
    }

    auto& GetPosition() const { return position_; }
    auto& GetVelocity() const { return offset_; }

    Rect GetRect() const { return Rect(position_, {TileSize, TileSize}); }

    MapCoordinate GetMapCorrdinate() const {
        Rect rect = GetRect();
        auto center = rect.Center();
        return {static_cast<int>(center.x / TileSize),
                static_cast<int>(center.y / TileSize)};
    }

    void Draw();
    virtual void Update();

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

    // void Update() override;

   private:
};

class Ghost : public Monster {
   public:
    Ghost(const Image& image, const Vector2& position, std::string name)
        : Monster(image, position), name_(name) {}

    static void InitAiMap();
    void Update() override;

    using AIType = std::function<Direction(Pacman&, Ghost&)>;

   private:
    std::string name_;
    inline static std::unordered_map<std::string, AIType> aiMap_ =
        std::unordered_map<std::string, AIType>();
    static AIType aiBlinky_;
};
