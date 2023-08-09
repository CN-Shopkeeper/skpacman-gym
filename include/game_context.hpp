#pragma once

#include "controller.hpp"
#include "map.hpp"
#include "monster.hpp"
#include "pch.hpp"

class GameContext final : public Singlton<GameContext> {
   public:
    bool debugMode = true;
    bool ShouldClose() const { return shouldClose_; }
    void Exit() { shouldClose_ = true; }

    SDL_Event& GetEvent() { return event_; }

    GameContext();

    void HandleEvent() {
        if (SDL_KEYDOWN == event_.type) {
            auto key = event_.key.keysym.scancode;
            if (SDL_SCANCODE_R == key) {
                newGame();
            }
            if (SDL_SCANCODE_G == key) {
                debugMode = !debugMode;
            }
            if (SDL_SCANCODE_M == key) {
                if (debugMode) {
                    modeCount_++;
                    auto nowMode = static_cast<Ghost::Mode>(modeCount_ % 3);
                    std::cout << nowMode << std::endl;
                    for (int i = 1; i < monsters.size(); i++) {
                        auto monster = monsters[i].get();
                        dynamic_cast<Ghost*>(monster)->ChangeMode(nowMode);
                    }
                }
            }
        }
        controller->HandleEvent(event_);
    }

    std::unique_ptr<Map> gameMap;
    std::vector<std::unique_ptr<Monster>> monsters;
    std::unique_ptr<Controller> controller;

    void Update();

   private:
    bool shouldClose_ = false;
    SDL_Event event_;
    int modeCount_ = 0;

    void newGame();
    void dealCollideWithMap(Monster& Monster);
};