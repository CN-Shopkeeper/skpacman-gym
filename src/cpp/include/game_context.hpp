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
    // 游戏正常运行的帧数
    int normalRunningFrame = 0;
    // 全局过去的帧数，用于对贴图进行更新
    int globalFrame = 0;

    std::array<EasterEgg, 4> easterEggInfo;

    bool ShouldClose() const { return shouldClose_; }
    void Exit() { shouldClose_ = true; }

    SDL_Event& GetEvent() { return event_; }

    GameContext();

    void HandleEvent() {
        if (SDL_KEYDOWN == event_.type) {
            auto key = event_.key.keysym.scancode;
            scancodeQueue_.push(key);
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

    int GetElapsedFloor() const {
        return static_cast<int>(
            std::floor(normalRunningFrame * FrameTime / 1000));
    }

    int GetRemainingLife() const { return lifeRemaining_; }

    void UpdateDebugText() {
        Ghost* ghost = dynamic_cast<Ghost*>(monsters[1].get());
        debugText.reset(Context::GetInstance().GenerateTextTexture(
            "Global Chrono: " + std::to_string(globalFrame * FrameTime / 1000) +
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
    int energizedFrame_ = 0;
    int multiKillReward_ = MultiKillReward;
    // 全局时间戳
    // TimePoint globalTime_ = std::chrono::system_clock::now();
    // // 上一帧的时间戳
    // TimePoint frameTime_ = globalTime_;

    FixedSizeQueue<SDL_Scancode, 6> scancodeQueue_;

    void initEasterEggInfo();
    void dealCollideWithMap(Monster& Monster);
    int tryCapture();
    bool tryEatBean();
};