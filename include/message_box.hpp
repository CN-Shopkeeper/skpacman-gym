#pragma once

#include "pch.hpp"

enum class MessageBoxResult { No = 0, Yes = 1, Cancel = 2 };

inline MessageBoxResult ShowMessageBox(const char* title, const char* message) {
    const SDL_MessageBoxButtonData buttons[] = {
        {/* .flags, .buttonid, .text */ 0,
         static_cast<int>(MessageBoxResult::No), "no"},
        {SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT,
         static_cast<int>(MessageBoxResult::Yes), "yes"},
        {SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT,
         static_cast<int>(MessageBoxResult::Cancel), "cancel"},
    };
    const SDL_MessageBoxColorScheme colorScheme = {
        {/* .colors (.r, .g, .b) */
         /* [SDL_MESSAGEBOX_COLOR_BACKGROUND] */
         {255, 0, 0},
         /* [SDL_MESSAGEBOX_COLOR_TEXT] */
         {0, 255, 0},
         /* [SDL_MESSAGEBOX_COLOR_BUTTON_BORDER] */
         {255, 255, 0},
         /* [SDL_MESSAGEBOX_COLOR_BUTTON_BACKGROUND] */
         {0, 0, 255},
         /* [SDL_MESSAGEBOX_COLOR_BUTTON_SELECTED] */
         {255, 0, 255}}};
    const SDL_MessageBoxData messageboxdata = {
        SDL_MESSAGEBOX_INFORMATION, /* .flags */
        NULL,                       /* .window */
        title,                      /* .title */
        message,                    /* .message */
        SDL_arraysize(buttons),     /* .numbuttons */
        buttons,                    /* .buttons */
        &colorScheme                /* .colorScheme */
    };
    int buttonid;
    if (SDL_ShowMessageBox(&messageboxdata, &buttonid) < 0) {
        SDL_Log("error displaying message box");
        throw std::runtime_error("error button id in message box");
    }
    if (buttonid == -1) {
        SDL_Log("no selection");
    } else {
        SDL_Log("selection was %s", buttons[buttonid].text);
    }
    return static_cast<MessageBoxResult>(buttonid);
}