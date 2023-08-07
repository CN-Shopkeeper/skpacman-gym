#pragma once

#include "pch.hpp"

constexpr SDL_Color KeyColor = {118, 66, 138, 255};
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

constexpr size_t tetrisHeight = 9, tetrisWidth = 5;

constexpr int MapWidth = tetrisWidth * 3 * 2;
constexpr int MapHeight = tetrisHeight * 3 + 2;

constexpr size_t TileSize = 24;

constexpr size_t WindowHeight = TileSize * MapHeight;
constexpr size_t WindowWidth = TileSize * MapWidth;

constexpr size_t PacmanInitX = WindowWidth / 2 - TileSize;
constexpr size_t PacmanInitY = (7 * 3 + 1) * TileSize;

constexpr size_t GhostInitX = WindowWidth / 2 - TileSize * 2;
constexpr size_t GhostInitY = (4 * 3 + 1) * TileSize;
