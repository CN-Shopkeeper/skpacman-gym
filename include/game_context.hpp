#pragma once

#include "context.hpp"
#include "controller.hpp"
#include "map.hpp"
#include "monster.hpp"
#include "pch.hpp"

class GameContext final : public Singlton<GameContext> {
   public:
    using TimePoint = std::chrono::system_clock::time_point;
    enum GameState { Gaming, Paused, Win, Gameover };
    GameState state = Gaming;
    bool DebugMode = false;
    std::unique_ptr<TextTexture> gameInfoText;
    std::unique_ptr<TextTexture> debugText;
    Texture* winImage;
    Texture* gameoverImage;
    // 游戏正常运行的时间
    float normalRunningElapsed = 0;
    // 全局过去的时间，用于对贴图进行更新
    float globalElapsed = 0;
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
                DebugMode = !DebugMode;
            }
            if (SDL_SCANCODE_M == key) {
                if (DebugMode) {
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

    int GetBeanEaten() const { return beanCount_ - beanLeft_; }

    int GetElapsedFloor() const { return std::floor(normalRunningElapsed); }

    void UpdateDebugText() {
        Ghost* ghost = dynamic_cast<Ghost*>(monsters[1].get());
        debugText.reset(Context::GetInstance().GenerateTextTexture(
            "Global Chrono: " + std::to_string(globalElapsed) +
            "\nMultiKill Reward: " + std::to_string(multiKillReward_) +
            "\nGhost mode:\n" + ghost->GetModeStr()));
    }

   private:
    bool shouldClose_ = false;
    SDL_Event event_;
    int beanCount_ = 0;
    int beanLeft_ = 0;
    int score_ = 0;
    int modeCount_ = 0;
    float energizedTime_ = 0.0f;
    int multiKillReward_ = MultiKillReward;
    // 全局时间戳
    TimePoint globalTime_ = std::chrono::system_clock::now();
    // 上一帧的时间戳
    TimePoint frameTime_ = globalTime_;

    void newGame();
    void dealCollideWithMap(Monster& Monster);
    void tryCapture();
    void tryEatBean();

    void updateGameInfoText() {
        gameInfoText.reset(Context::GetInstance().GenerateTextTexture(
            "Chrono: " + std::to_string(GetElapsedFloor()) + "\nBean Left:\n" +
            std::to_string(beanLeft_) + "\nScore:\n" + std::to_string(score_)));
    }
};