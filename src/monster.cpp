#include "monster.hpp"

void Monster::Draw() {
    Vector2 scale;
    float rotation = 0;
    scale.x = dir == Direction::Left ? -1 : 1;
    scale.y = 1;
    if (dir == Direction::Up) {
        rotation = -90;
    }
    if (dir == Direction::Down) {
        rotation = 90;
    }

    auto& renderer = Context::GetInstance().GetRenderer();
    renderer.DrawImage(image, position_, scale * Scale, rotation);
}

void Monster::Update() {
    if (dir == Direction::Left) {
        Move({-speed, 0});
    }
    if (dir == Direction::Right) {
        Move({speed, 0});
    }
    if (dir == Direction::Up) {
        Move({0, -speed});
    }
    if (dir == Direction::Down) {
        Move({0, speed});
    }
}
