#pragma once

#include "fixed_size.hpp"
#include "pch.hpp"
#include "texture.hpp"

class EasterEgg {
   public:
    std::unique_ptr<TextTexture> text;
    Vector2 position;
    bool show = false;
    std::array<SDL_Scancode, 6> scancodes;

    EasterEgg() {}

    EasterEgg(int index, TextTexture* _text) {
        text.reset(_text);
        switch (index) {
            case 0:
                scancodes = UpRightScancode;
                position.x =
                    TileSize * MapWidth /* - TileSize * 1 */ - text->rect.w;
                position.y = 1 * TileSize;
                break;
            case 1:
                scancodes = UpLeftScancode;
                position = {1 * TileSize, 1 * TileSize};
                break;
            case 2:
                scancodes = DownRightScancode;
                position.x =
                    TileSize * MapWidth /* - TileSize * 1*/ - text->rect.w;
                position.y = TileSize * MapHeight - TileSize * 1 - text->rect.h;
                break;
            case 3:
                scancodes = DownLeftScancode;
                position.x = 1 * TileSize;
                position.y = TileSize * MapHeight - TileSize * 1 - text->rect.h;
                break;
            default:
                break;
        }
    }

    bool Check(const FixedSizeQueue<SDL_Scancode, 6>& scancodeQueue) {
        if (show) return false;
        if (scancodeQueue.IsEqual(scancodes)) {
            show = true;
            return true;
        }
        return false;
    }
};