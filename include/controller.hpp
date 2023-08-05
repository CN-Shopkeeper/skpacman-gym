#pragma ocne

#include "monster.hpp"

class Controller final {
   public:
    Controller(Pacman& pacman) : pacman_(pacman) {}

    void HandleEvent(const SDL_Event& event);

   private:
    void moveUp();
    void moveDown();
    void moveRight();
    void moveLeft();

    Pacman& pacman_;
};