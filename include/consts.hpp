#pragma once

#include "pch.hpp"

constexpr SDL_Color KeyColor = {118, 66, 138, 255};
constexpr SDL_Color BlinkyColor = {255, 0, 0, 255};
constexpr SDL_Color PinkyColor = {255, 192, 203, 255};
constexpr SDL_Color InkyColor = {0, 255, 255, 255};
constexpr SDL_Color ClydeColor = {255, 127, 80, 255};
constexpr SDL_Color FrightenedColor = {0, 0, 255, 255};
constexpr SDL_Color InvincibleColor = {255, 0, 0, 255};
constexpr SDL_Color WhiteColor = {255, 255, 255, 255};

constexpr float Scale = 1.5;
inline std::string TilesheetName = "tilesheet";
constexpr int TilesheetCol = 7;
constexpr int TilesheetRow = 1;

enum class ImageTileType {
    Wall = 0,
    Pacman,
    PacmanEat,
    Bean,
    PowerBean,
    Ghost,
    GhostDoor,
};

// NOTE: edit your map here
constexpr std::string_view MapDesc =
    "WWWWWWWWWW"
    "WBBBBBBBBW"
    "WWBWGWBWWW"
    "WPBW WBWPW"
    "WPBWWWBWPW"
    "WWBBBBBBBW"
    "WWBBBBBBBW"
    "WWBBBBBBBW"
    "WWBBBBBBBW"
    "WWBBBBBBBW"
    "WWBBBBBBBW"
    "WWBBBBBBBW"
    "WWWWWWWWWW";

constexpr std::string_view Tips =
    "Tips:\n"
    "WASD -> Action\n"
    "R -> Restart\n"
    "P -> Pause\n";

// 界面相关

constexpr size_t tetrisHeight = 9, tetrisWidth = 5;

constexpr int MapWidth = tetrisWidth * 3 * 2;
constexpr int MapHeight = tetrisHeight * 3 + 2;

constexpr size_t TileSize = 24;
constexpr size_t TipsWidth = 160;
constexpr size_t RankingListWidth = 160;

constexpr size_t WindowHeight = TileSize * MapHeight;
constexpr size_t WindowWidth =
    TileSize * MapWidth + TipsWidth + RankingListWidth;

constexpr size_t PacmanInitX = (TileSize * MapWidth) / 2 - TileSize;
constexpr size_t PacmanInitY = (7 * 3 + 1) * TileSize;

constexpr size_t GhostInitX = (TileSize * MapWidth) / 2 - TileSize * 2;
constexpr size_t GhostInitY = (4 * 3 + 1) * TileSize;

// 游戏机制相关

constexpr float FrightenedTime = 15.0f;
constexpr int BeanScore = 10;
constexpr int MultiKillReward = 200;
constexpr int TimeBonusPerSec = 300;
constexpr int RemainingLifeBonus = 1000;
constexpr int BonusTimeCount = 30;
constexpr int RemainingLifeCount = 2;
constexpr float InvinciateTime = 3.0f;

// 彩蛋相关

// 彩蛋按键

// 左上角
constexpr std::array<SDL_Scancode, 6> UpLeftScancode = {
    SDL_SCANCODE_W, SDL_SCANCODE_A, SDL_SCANCODE_W,
    SDL_SCANCODE_A, SDL_SCANCODE_W, SDL_SCANCODE_A};
constexpr std::array<SDL_Scancode, 6> UpRightScancode = {
    SDL_SCANCODE_W, SDL_SCANCODE_D, SDL_SCANCODE_W,
    SDL_SCANCODE_D, SDL_SCANCODE_W, SDL_SCANCODE_D};
constexpr std::array<SDL_Scancode, 6> DownLeftScancode = {
    SDL_SCANCODE_S, SDL_SCANCODE_A, SDL_SCANCODE_S,
    SDL_SCANCODE_A, SDL_SCANCODE_S, SDL_SCANCODE_A};
constexpr std::array<SDL_Scancode, 6> DownRightScancode = {
    SDL_SCANCODE_S, SDL_SCANCODE_D, SDL_SCANCODE_S,
    SDL_SCANCODE_D, SDL_SCANCODE_S, SDL_SCANCODE_D};

// 内测人员名单
constexpr std::string_view testerList[] = {"shear and his Queen",
                                           "colin_008 & satori", "TX7 & Lynn00",
                                           "Crystal & M-thor"};
constexpr int TextWidth[] = {152, 144, 96, 128};