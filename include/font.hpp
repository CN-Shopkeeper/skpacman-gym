#pragma once
#include "pch.hpp"

inline auto FontDestroy = [](TTF_Font* font) { TTF_CloseFont(font); };

class Font final {
   private:
    std::unique_ptr<TTF_Font, decltype(FontDestroy)> font_;

   public:
    friend class Context;
    Font(const std::string& fontPath, int fontSize);
};