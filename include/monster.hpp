#pragma once

#include "consts.hpp"
#include "context.hpp"
#include "pch.hpp"
#include "texture.hpp"

class Monster final {
   public:
    enum class Direction { Right = 0, Left, Up, Down };

    Direction dir = Direction::Right;

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

    void Draw();
    void Update();

   private:
    Vector2 offset_;
    Vector2 position_;
};