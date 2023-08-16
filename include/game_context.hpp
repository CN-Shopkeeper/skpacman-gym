#pragma once

#include "context.hpp"
#include "controller.hpp"
#include "map.hpp"
#include "monster.hpp"
#include "pch.hpp"

class GameContext final : public Singlton<GameContext> {
   public:
    using TimePoint = std::chrono::_V2::system_clock::time_point;
    enum GameState { Gaming, Paused, Win, Gameover };
    GameState state = Gaming;
    bool debugMode = false;
    std::unique_ptr<TextTexture> scoreText;
    Texture* winImage;
    Texture* gameoverImage;
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
            if (SDL_SCANCODE_P == key) {
                state = static_cast<GameState>(1 - static_cast<int>(state));
            }
            if (GameState::Gaming == state) {
                controller->HandleEvent(event_);
            }
        }
    }

    std::unique_ptr<Map> gameMap;
    std::vector<std::unique_ptr<Monster>> monsters;
    std::unique_ptr<Controller> controller;

    void Update();

   private:
    bool shouldClose_ = false;
    SDL_Event event_;
    int beanCount_ = 0;
    int score_ = 0;
    int modeCount_ = 0;
    TimePoint startTime_ = std::chrono::system_clock::now();
    int elapsed = 0;

    void newGame();
    void dealCollideWithMap(Monster& Monster);
    void tryCapture();
    void tryEatBean();

    void updateGameInfoText() {
        scoreText.reset(Context::GetInstance().GenerateTextTexture(
            "Chrono: " + std::to_string(elapsed) + "\nBean Total:\n" +
            std::to_string(beanCount_) + "\nScore:\n" +
            std::to_string(score_)));
    }
};