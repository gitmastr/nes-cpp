#pragma once
#include "types.hpp"
#include "SDL2/SDL.h"

namespace Input
{
    namespace Controller
    {
        u8 value();
        void write(u8 value);
        u8 read();
    }

    void handle_event(const SDL_Event &event);
    u8 value();
}