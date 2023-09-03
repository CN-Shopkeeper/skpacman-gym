#pragma once
#include "font.hpp"
#include "pch.hpp"
#include "ranking_list.hpp"
#include "renderer.hpp"
#include "singlton.hpp"
#include "texture.hpp"
#include "window.hpp"

class Context final : public Singlton<Context> {
   public:
    Font font;
    std::unique_ptr<TextTexture> tips;
    using TimePoint = std::chrono::system_clock::time_point;

    TimePoint lastFrame =
        std::chrono::system_clock::from_time_t(std::time_t{0});

    Context(const std::string& title, const Size& windowSize);
    auto& GetRenderer() { return *renderer_; }
    auto& GetTextureManager() { return *textureManager_; }
    TextTexture* GenerateTextTexture(const std::string&,
                                     SDL_Color color = {255, 255, 255, 255},
                                     Uint32 wrapLength = 160);

   private:
    std::unique_ptr<Window> window_;
    std::unique_ptr<Renderer> renderer_;
    std::unique_ptr<TextureManager> textureManager_;
};
