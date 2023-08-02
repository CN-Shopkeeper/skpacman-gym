#include "context.hpp"

constexpr SDL_Color NormalTileColor = {150, 150, 150, 255};
constexpr SDL_Color HoverTileColor = {200, 200, 200, 255};
constexpr SDL_Color HightlightTileColor = {250, 250, 250, 255};
constexpr SDL_Color BorderTileColor = {0, 0, 0, 255};
constexpr SDL_Color NakedTileColor = {100, 100, 100, 255};
constexpr SDL_Color KeyColor = {118, 66, 138, 255};

std::unique_ptr<Context> Context::instance_ = nullptr;

SDL_Texture* LoadTexture(SDL_Renderer* renderer, const std::string& bmpFilename,
                         const SDL_Color& keycolor) {
    SDL_Surface* surface = SDL_LoadBMP(bmpFilename.c_str());
    if (!surface) {
        SDL_LogError(SDL_LOG_CATEGORY_RESERVED1, "%s load failed",
                     bmpFilename.c_str());
    }
    SDL_SetColorKey(
        surface, SDL_TRUE,
        SDL_MapRGB(surface->format, keycolor.r, keycolor.g, keycolor.b));
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        SDL_LogError(SDL_LOG_CATEGORY_RESERVED1,
                     "texture create from %s failed", bmpFilename.c_str());
    }
    SDL_FreeSurface(surface);
    return texture;
}

inline SDL_Point calcTileCoord(int mouseX, int mouseY) {
    return {mouseX / TileLen, mouseY / TileLen};
}

// 使用stack而非递归实现
void floodFill(Context& ctx, Map& map, int x, int y) {
    if (!map.IsIn(x, y)) {
        return;
    }
    auto tile = map.Get(x, y);
    if (!tile.isVisiable && !tile.isFlaged) {
        std::stack<SDL_Point> s;
        s.push({x, y});
        while (!s.empty()) {
            auto point = s.top();
            s.pop();
            auto& tile_ = map.Get(point.x, point.y);
            if (!tile_.isVisiable && !tile_.isFlaged) {
                tile_.isVisiable = true;
                ctx.nakedCount++;
            } else {
                continue;
            }
            if (tile_.value == 0) {
                // queen
                for (int dx = -1; dx <= 1; dx++) {
                    for (int dy = -1; dy <= 1; dy++) {
                        if (dx == 0 && dy == 0) {
                            continue;
                        }
                        int detectX = point.x + dx;
                        int detectY = point.y + dy;
                        if (map.IsIn(detectX, detectY)) {
                            s.push({detectX, detectY});
                        }
                    }
                }
                // rook
                // if (map.IsIn(point.x - 1, point.y)) {
                //     s.push({point.x - 1, point.y});
                // }
                // if (map.IsIn(point.x + 1, point.y)) {
                //     s.push({point.x + 1, point.y});
                // }
                // if (map.IsIn(point.x, point.y - 1)) {
                //     s.push({point.x, point.y - 1});
                // }
                // if (map.IsIn(point.x, point.y + 1)) {
                //     s.push({point.x, point.y + 1});
                // }
            }
        }
    }
}

Context::Context(Window&& window, Renderer&& renderer, Font&& font, Map&& map,
                 int mineCount)
    : numberImage(
          LoadTexture(renderer.renderer_.get(), "resources/font.bmp", KeyColor),
          TextureDestroy),
      mineImage(
          LoadTexture(renderer.renderer_.get(), "resources/mine.bmp", KeyColor),
          TextureDestroy),
      flagImage(
          LoadTexture(renderer.renderer_.get(), "resources/flag.bmp", KeyColor),
          TextureDestroy),
      gameoverImage(LoadTexture(renderer.renderer_.get(),
                                "resources/gameover.bmp", KeyColor),
                    TextureDestroy),
      winImage(
          LoadTexture(renderer.renderer_.get(), "resources/win.bmp", KeyColor),
          TextureDestroy),
      window(std::move(window)),
      renderer(std::move(renderer)),
      font(std::move(font)),
      map(std::move(map)),
      mineCount(mineCount) {}

void Context::handleMouseLeftBtnDown(const SDL_Point& p) {
    auto tileCoord = calcTileCoord(p.x, p.y);
    if (!map.IsIn(tileCoord.x, tileCoord.y)) {
        return;
    }

    auto& tile = map.Get(tileCoord.x, tileCoord.y);
    if (tile.isVisiable || tile.isFlaged) {
        return;
    }

    if (!tile.isVisiable && tile.type == Tile::Mine) {
        state = GameState::Explode;
        for (int i = 0; i < map.MaxSize(); i++) {
            auto& tile = map.GetByIndex(i);
            tile.isVisiable = true;
            tile.isFlaged = false;
        }
        return;
    }

    floodFill(*this, map, tileCoord.x, tileCoord.y);

    if (nakedCount == map.MaxSize() - mineCount) {
        state = GameState::Win;
    }
}

void Context::handleMouseRightBtnDown(const SDL_Point& p) {
    auto tileCoord = calcTileCoord(p.x, p.y);
    if (!map.IsIn(tileCoord.x, tileCoord.y)) {
        return;
    }

    auto& tile = map.Get(tileCoord.x, tileCoord.y);

    if (!tile.isVisiable) {
        if (tile.isFlaged) {
            map.AddFlagCount(-1);
        } else {
            map.AddFlagCount(1);
        }
        tile.isFlaged = !tile.isFlaged;
    }
}

void Context::handleKeyDown(const SDL_KeyboardEvent& e) {
    if (e.keysym.scancode == SDL_SCANCODE_G) {
        debugMode_ = !debugMode_;
    }
}

void Context::handleMouseBothPressed(const SDL_Point& p) {
    auto tileCoord = calcTileCoord(p.x, p.y);
    if (!map.IsIn(tileCoord.x, tileCoord.y)) {
        return;
    }
    auto& tile = map.Get(tileCoord.x, tileCoord.y);
    if (!tile.isVisiable) {
        return;
    }
    map.SetHighlight(calcTileCoord(mouse.Position().x, mouse.Position().y));
}

void Context::handleMouseBothPressing(const SDL_Point& p) {
    auto hightlightOpt = map.GetHightlight();
    if (hightlightOpt.has_value()) {
        auto hightlight = hightlightOpt.value();
        auto tileCoord = calcTileCoord(p.x, p.y);
        if (!map.IsIn(tileCoord.x, tileCoord.y) ||
            (hightlight.x != tileCoord.x || hightlight.y != tileCoord.y)) {
            map.RemoveHeight();
            return;
        }
    }
}

void Context::handleMouseBothReleased(const SDL_Point& p) {
    auto hightlightOpt = map.GetHightlight();
    if (hightlightOpt.has_value()) {
        auto hightlight = hightlightOpt.value();
        auto tileCoord = calcTileCoord(p.x, p.y);
        if (!map.IsIn(tileCoord.x, tileCoord.y) ||
            (hightlight.x != tileCoord.x || hightlight.y != tileCoord.y)) {
            map.RemoveHeight();
            return;
        }
        auto& center = map.Get(tileCoord.x, tileCoord.y);
        if (!center.isVisiable) {
            map.RemoveHeight();
            return;
        }
        int flagCount = 0;
        for (int dx = -1; dx <= 1; dx++) {
            for (int dy = -1; dy <= 1; dy++) {
                int detectX = tileCoord.x + dx;
                int detectY = tileCoord.y + dy;
                if (map.IsIn(detectX, detectY)) {
                    auto& tile_ = map.Get(detectX, detectY);
                    if (!tile_.isVisiable && tile_.isFlaged) {
                        flagCount++;
                    }
                }
            }
        }
        if (center.value == flagCount) {
            for (int dx = -1; dx <= 1; dx++) {
                for (int dy = -1; dy <= 1; dy++) {
                    int detectX = tileCoord.x + dx;
                    int detectY = tileCoord.y + dy;
                    if (map.IsIn(detectX, detectY)) {
                        auto& tile_ = map.Get(detectX, detectY);
                        if (!tile_.isVisiable && !tile_.isFlaged) {
                            if (tile_.type == Tile::Mine) {
                                state = GameState::Explode;
                                for (int i = 0; i < map.MaxSize(); i++) {
                                    auto& tile = map.GetByIndex(i);
                                    tile.isVisiable = true;
                                    tile.isFlaged = false;
                                }
                                return;
                            }
                            floodFill(*this, map, detectX, detectY);
                        }
                    }
                }
            }
            if (nakedCount == map.MaxSize() - mineCount) {
                state = GameState::Win;
            }
        }
        map.RemoveHeight();
    }
}

Map createRandomMap(int mineCount, int w, int h) {
    SDL_assert(mineCount < w * h);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution dist1(0, w - 1);
    std::uniform_int_distribution dist2(0, h - 1);

    Map map(w, h);

    constexpr int MaxMineSetupCount = 100;

    // generate all mines
    while (mineCount > 0) {
        bool setupFinish = false;
        int setupCount = 100;
        while (!setupFinish && setupCount > 0) {
            int x = dist1(gen);
            int y = dist2(gen);
            auto& tile = map.Get(x, y);
            if (tile.type != Tile::Mine) {
                tile.type = Tile::Mine;
                setupFinish = true;
            } else {
                setupCount--;
            }
        }

        if (setupCount == 0) {
            for (int i = 0; i < map.MaxSize(); i++) {
                if (!map.GetByIndex(i).type != Tile::Mine) {
                    map.GetByIndex(i).type = Tile::Mine;
                    break;
                }
            }
        }

        mineCount--;
    }

    // generate nearly mine number
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            auto& tile = map.Get(x, y);
            if (tile.type == Tile::Mine) {
                for (int dx = -1; dx <= 1; dx++) {
                    for (int dy = -1; dy <= 1; dy++) {
                        int detectX = x + dx;
                        int detectY = y + dy;
                        if (map.IsIn(detectX, detectY)) {
                            auto& detectTile = map.Get(detectX, detectY);
                            if (detectTile.type != Tile::Mine) {
                                detectTile.value++;
                            }
                        }
                    }
                }
            }
        }
    }

    return map;
}

void Context::drawOneTile(int x, int y, const Tile& tile) {
    auto tileX = x * TileLen;
    auto tileY = y * TileLen;
    SDL_Rect rect = {tileX, tileY, TileLen, TileLen};
    SDL_Point mousePos;

    // 先绘制底色
    SDL_GetMouseState(&mousePos.x, &mousePos.y);
    if (SDL_PointInRect(&mousePos, &rect)) {
        renderer.SetColor(HoverTileColor);
    } else {
        if (map.GetHightlight().has_value()) {
            auto hightlightCenter = map.GetHightlight().value();
            if (std::abs(hightlightCenter.x - x) <= 1 &&
                std::abs(hightlightCenter.y - y) <= 1 && !tile.isVisiable) {
                renderer.SetColor(HightlightTileColor);
            } else {
                if (tile.isVisiable) {
                    renderer.SetColor(NakedTileColor);
                } else {
                    renderer.SetColor(NormalTileColor);
                }
            }
        } else {
            if (tile.isVisiable) {
                renderer.SetColor(NakedTileColor);
            } else {
                renderer.SetColor(NormalTileColor);
            }
        }
    }
    renderer.FillRect(rect);
    renderer.SetColor(BorderTileColor);
    renderer.DrawRect(rect);

    // 再绘制纹理
    if (tile.isVisiable) {
        if (tile.type == Tile::Mine) {
            renderer.DrawTexture(mineImage.get(), SDL_Rect{0, 0, 32, 32}, tileX,
                                 tileY);
        } else {
            int mineCount = tile.value;
            if (mineCount > 0) {
                renderer.DrawTexture(
                    numberImage.get(),
                    SDL_Rect{309 / 8 * (mineCount - 1), 0, 32, 32}, tileX,
                    tileY);
            }
        }
    } else {
        if (tile.isFlaged) {
            renderer.DrawTexture(flagImage.get(), SDL_Rect{0, 0, 32, 32}, tileX,
                                 tileY);
        }
    }

    if (debugMode_) {
        if (tile.type == Tile::Mine) {
            renderer.DrawTexture(mineImage.get(), SDL_Rect{0, 0, 32, 32}, tileX,
                                 tileY);
        } else {
            int mineCount = tile.value;
            if (mineCount > 0) {
                renderer.DrawTexture(
                    numberImage.get(),
                    SDL_Rect{309 / 8 * (mineCount - 1), 0, 32, 32}, tileX,
                    tileY);
            }
        }
    }
}

void Context::DrawMap() {
    for (int y = 0; y < map.Height(); y++) {
        for (int x = 0; x < map.Width(); x++) {
            const auto& tile = map.Get(x, y);
            drawOneTile(x, y, tile);
        }
    }
    if (state == GameState::Explode) {
        renderer.DrawTexture(gameoverImage.get(), SDL_Rect{0, 0, 256, 256},
                             (WindowWidth - 256) / 2, (WindowHeight - 256) / 2);
    }
    if (state == GameState::Win) {
        renderer.DrawTexture(winImage.get(), SDL_Rect{0, 0, 256, 256},
                             (WindowWidth - 256) / 2, (WindowHeight - 256) / 2);
    }
}

void Context::HandleEvents(std::vector<SDL_Event>& events) {
    std::vector<SDL_Event> mouseEvents;
    for (auto& e : events) {
        if (e.type == SDL_KEYDOWN) {
            handleKeyDown(e.key);
        } else {
            // 全都放到mouseEvents中，让它筛选
            mouseEvents.push_back(e);
        }
    }
    mouse.UpdateMouse(mouseEvents);
    if (state != GameState::Gaming) {
        if (mouse.LeftBtn().IsPressed()) {
            map =
                createRandomMap(MineCount, WindowWidth / TileLen,
                                (WindowHeight - MessageTitleHeight) / TileLen);
            mineCount = MineCount;
            nakedCount = 0;
            state = GameState::Gaming;
        }
        return;
    }
    if (mouse.LeftBtn().IsPressing() && mouse.RightBtn().IsPressing()) {
        handleMouseBothPressing(mouse.Position());
    } else if (mouse.LeftBtn().IsPressed() && mouse.RightBtn().IsPressed()) {
        handleMouseBothPressed(mouse.Position());
    } else if (mouse.LeftBtn().IsReleased() && mouse.RightBtn().IsReleased()) {
        handleMouseBothReleased(mouse.Position());
    } else if (mouse.LeftBtn().IsReleasing() &&
               mouse.RightBtn().IsReleasing()) {
        handleMouseBothReleased(mouse.Position());
    } else {
        if (mouse.LeftBtn().IsPressed()) {
            handleMouseLeftBtnDown(mouse.Position());
        }
        if (mouse.RightBtn().IsPressed()) {
            handleMouseRightBtnDown(mouse.Position());
        }
    }
}

void Context::DrawMessage() {
    std::string message("地雷总数: ");
    message.append(std::to_string(mineCount))
        .append(" / ")
        .append("已插旗数: ")
        .append(std::to_string(map.GetFlagCount()));

    auto textSurface = TTF_RenderUTF8_Blended(font.font_.get(), message.c_str(),
                                              {0, 0, 0, 255});
    auto textTexture =
        SDL_CreateTextureFromSurface(renderer.renderer_.get(), textSurface);
    SDL_Rect rect = {0, 0, textSurface->w, textSurface->h};
    SDL_FreeSurface(textSurface);
    renderer.DrawTexture(textTexture, rect, 0,
                         WindowHeight - MessageTitleHeight * 0.75);
    SDL_DestroyTexture(textTexture);
}