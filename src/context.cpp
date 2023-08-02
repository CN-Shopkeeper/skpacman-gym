#include "context.hpp"

Context::Context(const std::string& title, const Size& windowSize) {
    window_.reset(new Window(title, static_cast<int>(windowSize.x),
                             static_cast<int>(windowSize.y)));
    renderer_.reset(new Renderer(*window_));
    textureManager_.reset(new TextureManager(renderer_.get()));
}