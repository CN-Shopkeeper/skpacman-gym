#include "font.hpp"

Font::Font(const std::string& fontPath, int fontSize)
    : font_(TTF_OpenFont(fontPath.c_str(), fontSize), FontDestroy) {}