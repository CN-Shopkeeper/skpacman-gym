#pragma once

#include "pch.hpp"

inline auto WindowDestroy = [](SDL_Window* data) { SDL_DestroyWindow(data); };

class Window final {
   private:
    std::unique_ptr<SDL_Window, decltype(WindowDestroy)> window_;

   public:
    friend class Renderer;
    Window(const std::string& title, int w, int h);
};