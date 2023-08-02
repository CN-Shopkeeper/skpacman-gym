#include "texture.hpp"

#include "renderer.hpp"

Texture::Texture(Renderer* renderer, const std::string& filename,
                 const SDL_Color& keyColor, std::optional<size_t> tilesheetIdx)
    : renderer_(renderer),
      texture_(nullptr, DestroyTexture),
      tilesheetIdx_(tilesheetIdx) {
    SDL_Surface* surface = SDL_LoadBMP(filename.c_str());
    if (!surface) {
        SDL_Log("[Texture]: load image %s failed: %s", filename.c_str(),
                SDL_GetError());
        texture_ = nullptr;
    } else {
        size_.Set(static_cast<float>(surface->w),
                  static_cast<float>(surface->h));
        SDL_SetColorKey(
            surface, SDL_TRUE,
            SDL_MapRGB(surface->format, keyColor.r, keyColor.g, keyColor.b));
        texture_.reset(
            SDL_CreateTextureFromSurface(renderer->renderer_.get(), surface));
        if (!texture_) {
            SDL_Log("[Texture]: create texture from %s failed: %s",
                    filename.c_str(), SDL_GetError());
        }
    }
}

Texture& TextureManager::Load(const std::string& name,
                              const std::string& filename,
                              const SDL_Color& keyColor) {
    return textures_
        .emplace(name, Texture(renderer_, filename, keyColor, std::nullopt))
        .first->second;
}

Texture* TextureManager::Find(const std::string& name) {
    if (auto it = textures_.find(name); it != textures_.end()) {
        // if (it->second.)
        return &it->second;
    }
    return nullptr;
}
