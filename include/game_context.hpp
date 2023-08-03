#pragma once

#include "pch.hpp"

class GameContext final : public Singlton<GameContext> {
   public:
    bool ShouldClose() const { return shouldClose_; }
    void Exit() { shouldClose_ = true; }

    SDL_Event& GetEvent() { return event_; }

    GameContext();

    void HandleEvent() {}

   private:
    bool shouldClose_ = false;
    SDL_Event event_;
};