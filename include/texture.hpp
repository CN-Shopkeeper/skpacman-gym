#pragma once

#include "pch.hpp"
#include "vmath.hpp"

inline auto DestroyTexture = [](SDL_Texture* texture) {
    SDL_DestroyTexture(texture);
};

class Renderer;

class Texture final {
   public:
    friend class Renderer;
    Texture(Renderer* renderer, const std::string& filename,
            const SDL_Color& keyColor, std::optional<size_t> tilesheetIdx);

   private:
    std::optional<size_t> tilesheetIdx_;

    Renderer* renderer_;
    Vector2 size_;

    std::unique_ptr<SDL_Texture, decltype(DestroyTexture)> texture_;
};

class TextureManager final {
   public:
    TextureManager(Renderer* renderer) : renderer_(renderer) {}
    TextureManager(const TextureManager&) = delete;
    TextureManager& operator=(const TextureManager&) = delete;

    Texture& Load(const std::string& name, const std::string& filename,
                  const SDL_Color& keyColor);
    Texture* Find(const std::string& name);

   private:
    Renderer* renderer_;
    std::unordered_map<std::string, Texture> textures_;
    // std::vector<T
};