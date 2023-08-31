#pragma once
#include "map.hpp"
#include "pch.hpp"
#include "texture.hpp"
#include "window.hpp"

inline auto RendererDestroy = [](SDL_Renderer* renderer) {
    SDL_DestroyRenderer(renderer);
};

class Renderer final {
   private:
    std::unique_ptr<SDL_Renderer, decltype(RendererDestroy)> renderer_;

   public:
    friend class Texture;
    friend class Context;
    Renderer(const Window&);

    void SetColor(const SDL_Color& color);
    void Clear();
    void Present();
    void DrawRect(const SDL_Rect& rect);
    void FillRect(const SDL_Rect& rect);
    void DrawLine(const SDL_Point& p1, const SDL_Point& p2);
    void DrawTexture(Texture& texture, const SDL_Rect& rect, int x, int y);
    void DrawImage(const Image& image, const Vector2& position,
                   const std::optional<Vector2>& size);
    void DrawImage(const Image& image, const Vector2& position,
                   const Vector2 scale, float rotation = 0);
    void DrawPath(const std::unique_ptr<SDL_Point[]>& path,
                  const SDL_Color& color, int count);
    void DrawTextTexture(TextTexture& textTexture, int x, int y);
};