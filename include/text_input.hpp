#pragma once

#include "pch.hpp"

template <size_t MaxSize>
class TextInputHandler {
   public:
    bool canInput = true;

    void HandleTextInput(SDL_Event &event) {
        std::cout << "text input " << event.text.text << std::endl;
        for (int i = 0; event.text.text[i] != '\0'; i++) {
            textInputStack_.push(event.text.text[i]);
        }
        std::cout << "stack: " << textInputStack_.ToString() << std::endl
                  << std::endl;
    }

    void PopOneChar() {
        textInputStack_.Pop();
        std::cout << "stack: " << textInputStack_.ToString() << std::endl;
    }

   private:
    FixedSizeStack<char, MaxSize> textInputStack_;
};
