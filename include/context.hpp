#pragma once
#include "font.hpp"
#include "matrix.hpp"
#include "mouse.hpp"
#include "pch.hpp"
#include "renderer.hpp"
#include "window.hpp"

class Tile {
   public:
    enum Type { Mine = -1 };
    union {
        Type type;
        int value = 0;
    };
    bool isVisiable = false;
    bool isFlaged = false;
};

constexpr int TileLen = 32;
constexpr int WindowWidth = TileLen * 20;
constexpr int MessageTitleHeight = 64;
constexpr int WindowHeight = TileLen * 20 + MessageTitleHeight;
constexpr int MineCount = 50;

using Map = Matrix<Tile>;

Map createRandomMap(int mineCount, int w, int h);

inline auto TextureDestroy = [](SDL_Texture* texture) {
    SDL_DestroyTexture(texture);
};

class Context final {
   private:
    enum GameState { Gaming, Win, Explode };
    static std::unique_ptr<Context> instance_;
    bool debugMode_ = false;

    Context(Window&& window, Renderer&& renderer, Font&& font, Map&& map,
            int mineCount);

    void drawOneTile(int x, int y, const Tile& tile);
    void handleMouseLeftBtnDown(const SDL_Point&);
    void handleMouseRightBtnDown(const SDL_Point&);
    void handleKeyDown(const SDL_KeyboardEvent&);
    void handleMouseBothPressed(const SDL_Point&);
    void handleMouseBothPressing(const SDL_Point&);
    void handleMouseBothReleased(const SDL_Point&);

   public:
    GameState state = Gaming;

    std::unique_ptr<SDL_Texture, decltype(TextureDestroy)> numberImage;
    std::unique_ptr<SDL_Texture, decltype(TextureDestroy)> mineImage;
    std::unique_ptr<SDL_Texture, decltype(TextureDestroy)> flagImage;
    std::unique_ptr<SDL_Texture, decltype(TextureDestroy)> gameoverImage;
    std::unique_ptr<SDL_Texture, decltype(TextureDestroy)> winImage;

    Window window;
    Renderer renderer;
    Font font;
    Map map;
    Mouse mouse;
    int nakedCount;
    int mineCount;

    static void Init() {
        if (!instance_) {
            Window window("mine-sweep", WindowWidth, WindowHeight);
            Renderer renderer(window);
            Font font("resources/font/simsun.ttc", 32);
            // 因为构造函数是private的，所以不能make_unique
            // instance_ = std::make_unique<Context>(std::move(window),
            //                                       std::move(renderer));
            instance_ = std::unique_ptr<Context>(new Context{
                std::move(window), std::move(renderer), std::move(font),
                createRandomMap(MineCount, WindowWidth / TileLen,
                                (WindowHeight - MessageTitleHeight) / TileLen),
                MineCount});
        }
    }

    static void Quit() { instance_.reset(); }

    static Context& Inst() {
        SDL_assert(instance_);
        return *instance_;
    }

    void DrawMap();
    // void HandleEvent(SDL_Event&);
    void HandleEvents(std::vector<SDL_Event>& events);

    void DrawMessage();
};
