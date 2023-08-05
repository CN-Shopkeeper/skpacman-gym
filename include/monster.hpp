#pragma once

#include "consts.hpp"
#include "context.hpp"
#include "pch.hpp"
#include "texture.hpp"

class Monster {
   public:
    enum class Direction { Right = 0, Down, Left, Up };

    Direction movingDir = Direction::Right;

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
    virtual void Update();

   protected:
    Vector2 offset_;
    Vector2 position_;
};

class Pacman : public Monster {
   public:
    Pacman(const Image& image, const Vector2& position)
        : Monster(image, position) {
        intentionDir = movingDir;
    }
    Direction intentionDir;

    void Update() override;

   private:
    bool isTurnBack() const;
};
