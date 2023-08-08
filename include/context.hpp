#pragma once
#include "font.hpp"
#include "pch.hpp"
#include "renderer.hpp"
#include "singlton.hpp"
#include "texture.hpp"
#include "window.hpp"

class Context final : public Singlton<Context> {
   public:
    Font font;
    std::unique_ptr<TextTexture> tips;
    Context(const std::string& title, const Size& windowSize);
    auto& GetRenderer() { return *renderer_; }
    auto& GetTextureManager() { return *textureManager_; }

   private:
    std::unique_ptr<Window> window_;
    std::unique_ptr<Renderer> renderer_;
    std::unique_ptr<TextureManager> textureManager_;
};
