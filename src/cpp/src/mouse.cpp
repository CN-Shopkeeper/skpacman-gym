#include "mouse.hpp"

void Mouse::UpdateMouse(std::vector<SDL_Event>& events) {
    for (auto& btn : buttons_) {
        btn.lastState = btn.isPress;
    }
    for (auto& event : events) {
        if (event.type == SDL_MOUSEMOTION) {
            auto motion = event.motion;
            position_.x = motion.x;
            position_.y = motion.y;
            offset_.x = motion.xrel;
            offset_.y = motion.yrel;
        }
        if (event.type == SDL_MOUSEBUTTONDOWN ||
            event.type == SDL_MOUSEBUTTONUP) {
            updateOneBtn(event.button);
        }
    }
}

void Mouse::updateOneBtn(const SDL_MouseButtonEvent& event) {
    auto& btn = buttons_[event.button - 1];
    btn.lastState = btn.isPress;
    if (event.type == SDL_MOUSEBUTTONDOWN) {
        btn.isPress = true;
    }
    if (event.type == SDL_MOUSEBUTTONUP) {
        btn.isPress = false;
    }
}