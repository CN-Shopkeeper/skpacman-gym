#include "context.hpp"

Context::Context(const std::string& title, const Size& windowSize)
    : font(Font("resources/font/simsun.ttc", 16)) {
    window_.reset(new Window(title, static_cast<int>(windowSize.x),
                             static_cast<int>(windowSize.y)));
    renderer_.reset(new Renderer(*window_));
    textureManager_.reset(new TextureManager(renderer_.get()));

    tips.reset(new TextTexture(renderer_->renderer_.get(), Tips.data(),
                               font.font_.get()));
}

TextTexture* Context::GenerateTextTexture(const std::string& text) {
    return new TextTexture(renderer_->renderer_.get(), text.c_str(),
                           font.font_.get());
}
