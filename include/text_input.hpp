#pragma once

#include "message_box.hpp"
#include "pch.hpp"

template <size_t MaxSize>
class TextInputHandler {
   public:
    bool canInput = false;
    bool finished = false;

    void HandleEvent(const SDL_Event &event) {
        if (event.type == SDL_KEYDOWN) {
            if (event.key.keysym.scancode == SDL_SCANCODE_RETURN) {
                if (textInputStack_.Size() == 0) {
                    ShowMessageBox("错误的", "用户名不能为空\n任意键返回");
                } else {
                    auto result = ShowMessageBox(
                        "输入ID", ("你的ID是" + GetContent() +
                                   "\nCancel取消录入\nYes确认录入\nNo继续录入")
                                      .c_str());
                    if (result == MessageBoxResult::Cancel) {
                        canInput = false;
                        SDL_StopTextInput();
                    } else if (result == MessageBoxResult::Yes) {
                        canInput = false;
                        SDL_StopTextInput();
                        finished = true;
                    } else if (result == MessageBoxResult::No) {
                        // do nothing. let player continue input
                    }
                }
            }
            if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
                auto result = ShowMessageBox("退出", "放弃此次游玩成绩？");
                if (result == MessageBoxResult::Cancel) {
                    // do nothing. let player continue input
                } else if (result == MessageBoxResult::Yes) {
                    canInput = false;
                    SDL_StopTextInput();
                    textInputStack_.Clear();
                } else if (result == MessageBoxResult::No) {
                    // do nothing. let player continue input
                }
                canInput = false;
                SDL_StopTextInput();
            }
            if (event.key.keysym.scancode == SDL_SCANCODE_BACKSPACE) {
                PopOneChar();
            }
        } else if (event.type == SDL_TEXTINPUT) {
            handleTextInput(event);
        }
    }

    void PopOneChar() {
        textInputStack_.Pop();
        std::cout << "stack: " << textInputStack_.ToString() << std::endl;
    }

    std::string GetContent() const { return textInputStack_.ToString(); }

   private:
    FixedSizeStack<char, MaxSize> textInputStack_;

    void handleTextInput(const SDL_Event &event) {
        std::cout << "text input " << event.text.text << std::endl;
        for (int i = 0; event.text.text[i] != '\0'; i++) {
            textInputStack_.push(event.text.text[i]);
        }
        std::cout << "stack: " << textInputStack_.ToString() << std::endl
                  << std::endl;
    }
};
