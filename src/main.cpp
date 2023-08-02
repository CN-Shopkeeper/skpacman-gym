#include <iostream>

#include "context.hpp"

int main(int argc, char** argv) {
    SDL_Init(SDL_INIT_EVERYTHING);
    TTF_Init();
    // Context ::Init();

    // auto& ctx = Context::Inst();
    // auto& renderer = ctx.renderer;
    // SDL_Event event;
    // bool shouldClose = false;
    // std::vector<SDL_Event> events;
    // while (!shouldClose) {
    //     events.clear();
    //     while (SDL_PollEvent(&event)) {
    //         if (event.type == SDL_QUIT) {
    //             shouldClose = true;
    //         }
    //         events.push_back(event);
    //     }
    //     ctx.HandleEvents(events);

    //     renderer.SetColor(SDL_Color{200, 200, 200, 255});
    //     renderer.Clear();
    //     ctx.DrawMap();
    //     ctx.DrawMessage();

    //     renderer.Present();

    //     SDL_Delay(50);
    // }

    // Context::Quit();
    TTF_Quit();
    SDL_Quit();
    return 0;
}