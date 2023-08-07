#include "controller.hpp"

void Controller::moveUp() { pacman.intentionDir = Monster::Direction::Up; }

void Controller::moveDown() { pacman.intentionDir = Monster::Direction::Down; }

void Controller::moveRight() {
    pacman.intentionDir = Monster::Direction::Right;
}

void Controller::moveLeft() { pacman.intentionDir = Monster::Direction::Left; }

void Controller::HandleEvent(const SDL_Event& event) {
    if (SDL_KEYDOWN == event.type) {
        auto key = event.key.keysym.scancode;
        if (SDL_SCANCODE_A == key) {
            moveLeft();
        }
        if (SDL_SCANCODE_D == key) {
            moveRight();
        }
        if (SDL_SCANCODE_W == key) {
            moveUp();
        }
        if (SDL_SCANCODE_S == key) {
            moveDown();
        }
    }
}
