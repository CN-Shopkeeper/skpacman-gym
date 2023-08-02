#include "pch.hpp"

template <typename T>
struct Button {
    T btn;
    bool isPress = false;
    bool lastState = false;

    bool IsPressed() const { return !lastState && isPress; }
    bool IsReleased() const { return lastState && !isPress; }
    bool IsPressing() const { return lastState && isPress; }
    bool IsReleasing() const { return !lastState && !isPress; }
};

using MouseButton = Button<uint8_t>;

class Mouse {
   public:
    void UpdateMouse(std::vector<SDL_Event>& events);

    SDL_Point Position() const { return position_; }
    SDL_Point Offset() const { return offset_; }
    const MouseButton& LeftBtn() const { return buttons_[0]; }
    const MouseButton& MiddleBtn() const { return buttons_[1]; }
    const MouseButton& RightBtn() const { return buttons_[2]; }

   private:
    SDL_Point position_;
    SDL_Point offset_;
    MouseButton buttons_[3] = {MouseButton{SDL_BUTTON_LEFT},
                               MouseButton{SDL_BUTTON_MIDDLE},
                               MouseButton{SDL_BUTTON_RIGHT}};

    void updateOneBtn(const SDL_MouseButtonEvent&);
};
