#pragma once

#include "pch.hpp"

constexpr SDL_Color KeyColor = {118, 66, 138, 255};
constexpr SDL_Color BlinkyColor = {255, 0, 0, 255};
constexpr SDL_Color PinkyColor = {255, 192, 203, 255};
constexpr SDL_Color InkyColor = {0, 255, 255, 255};
constexpr SDL_Color ClydeColor = {255, 127, 80, 255};
constexpr SDL_Color FrightenedColor = {0, 0, 255, 255};

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
    "操作提示:\n"
    "WASD->行走\n"
    "R->重开游戏\n"
    "G->开发者模式\n"
    "M(G模式下)->\n"
    "更改Ghost模式\n"
    "P->暂停\n";

// 界面相关

constexpr size_t tetrisHeight = 9, tetrisWidth = 5;

constexpr int MapWidth = tetrisWidth * 3 * 2;
constexpr int MapHeight = tetrisHeight * 3 + 2;

constexpr size_t TileSize = 24;
constexpr size_t TipsWidth = 128;

constexpr size_t WindowHeight = TileSize * MapHeight;
constexpr size_t WindowWidth = TileSize * MapWidth + TipsWidth;

constexpr size_t PacmanInitX = WindowWidth / 2 - TileSize;
constexpr size_t PacmanInitY = (7 * 3 + 1) * TileSize;

constexpr size_t GhostInitX = (TileSize * MapWidth) / 2 - TileSize * 2;
constexpr size_t GhostInitY = (4 * 3 + 1) * TileSize;
