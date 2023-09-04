#pragma once

#include "context.hpp"
#include "controller.hpp"
#include "easter_egg.hpp"
#include "map.hpp"
#include "monster.hpp"
#include "pch.hpp"

class GameContext final : public Singlton<GameContext> {
   public:
    using TimePoint = std::chrono::system_clock::time_point;
    enum GameState { Gaming, Paused, Win, Gameover };
    GameState state = Gaming;
    bool DebugMode = false;
    // Won 和 GameIsOver用于记录游戏获胜或失败的那一刻
    bool Won = false, GameIsOver = false;
    // 记录这一帧是否吃到了豆子
    bool eatABean = false;
    int captureResult = 0;
    std::unique_ptr<TextTexture> gameInfoText;
    std::unique_ptr<TextTexture> debugText;
    std::unique_ptr<TextTexture> rankingListText;
    Texture* winImage;
    Texture* gameoverImage;
    // 游戏正常运行的时间
    float normalRunningElapsed = 0;
    // 全局过去的时间，用于对贴图进行更新
    float globalElapsed = 0;

    std::array<EasterEgg, 4> easterEggInfo;

    bool ShouldClose() const { return shouldClose_; }
    void Exit() { shouldClose_ = true; }

    SDL_Event& GetEvent() { return event_; }

    GameContext();

    void HandleEvent() {
        if (SDL_KEYDOWN == event_.type) {
            auto key = event_.key.keysym.scancode;
            scancodeQueue_.push(key);
            if (SDL_SCANCODE_R == key) {
                NewGame();
            }
            if (SDL_SCANCODE_G == key) {
                // ! 移除Debug模式
                // DebugMode = !DebugMode;
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

    int GetScore() const { return score_; }

    void NewGame(std::optional<int> seed = std::nullopt);

    void CalculateScore() {
        score_ += GetTimeBonus();
        score_ += GetRemainingLifeBonus();
    }

    int GetBeanEaten() const { return beanCount_ - beanLeft_; }

    int GetElapsedFloor() const { return std::floor(normalRunningElapsed); }

    int GetRemainingLife() const { return lifeRemaining_; }

    void UpdateDebugText() {
        Ghost* ghost = dynamic_cast<Ghost*>(monsters[1].get());
        debugText.reset(Context::GetInstance().GenerateTextTexture(
            "Global Chrono: " + std::to_string(globalElapsed) +
            "\nMultiKill Reward: " + std::to_string(multiKillReward_) +
            "\nGhost mode:\n" + ghost->GetModeStr() + "\nRandom Seed:\n" +
            std::to_string(Tetris::randSeed)));
    }

    void TryEasterEgg();

    int GetTimeBonus() const {
        return std::max(0, BonusTimeCount - GetElapsedFloor()) *
               TimeBonusPerSec;
    }

    int GetBonusTime() const {
        return std::max(0, BonusTimeCount - GetElapsedFloor());
    }

    int GetRemainingLifeBonus() const {
        return lifeRemaining_ * RemainingLifeBonus;
    }

    std::array<int, MapWidth * MapHeight> GetMapTiles() const {
        std::array<int, MapWidth * MapHeight> mapTiles;
        for (int i = 0; i < MapWidth; i++) {
            for (int j = 0; j < MapHeight; j++) {
                auto& tile = gameMap->GetTile(i, j);
                switch (tile.type) {
                    case Tile::Type::Empty:
                        mapTiles[i + j * MapWidth] = 0;
                        break;
                    case Tile::Type::Bean:
                        mapTiles[i + j * MapWidth] = 1;
                        break;
                    case Tile::Type::PowerBean:
                        mapTiles[i + j * MapWidth] = 2;
                        break;
                    case Tile::Type::Wall:
                        mapTiles[i + j * MapWidth] = 3;
                        break;
                    case Tile::Type::GhostDoor:
                        mapTiles[i + j * MapWidth] = 4;
                        break;
                    default:
                        break;
                }
            }
        }
        return mapTiles;
    }

    void UpdateGameInfoText() {
        gameInfoText.reset(Context::GetInstance().GenerateTextTexture(
            std::string("Chrono: " + std::to_string(GetElapsedFloor()))
                .append("\nBean Left:\n" + std::to_string(beanLeft_))
                .append("\nScore:\n" + std::to_string(score_) + "+" +
                        std::to_string(GetTimeBonus()) + "+" +
                        std::to_string(GetRemainingLifeBonus()))
                .append("\n\nLifeRemaining")));
    }

    void UpdateRankingListText() {
        rankingListText.reset(Context::GetInstance().GenerateTextTexture(
            RankingList::GetInstance().ToString()));
    }

   private:
    bool shouldClose_ = false;
    SDL_Event event_;
    int beanCount_ = 0;
    int beanLeft_ = 0;
    int lifeRemaining_ = RemainingLifeCount;
    int score_ = 0;
    int modeCount_ = 0;
    float energizedTime_ = 0.0f;
    int multiKillReward_ = MultiKillReward;
    // 全局时间戳
    TimePoint globalTime_ = std::chrono::system_clock::now();
    // 上一帧的时间戳
    TimePoint frameTime_ = globalTime_;

    FixedSizeQueue<SDL_Scancode, 6> scancodeQueue_;

    void initEasterEggInfo();
    void dealCollideWithMap(Monster& Monster);
    int tryCapture();
    bool tryEatBean();
};