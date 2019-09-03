#pragma once
#include "types.hpp"
#include "SDL2/SDL.h"

namespace Input
{
    class Controller
    {
    private:
        vector<bool> buttons;
        u8 index;
        bool polling;
    public:
        Controller();
        virtual ~Controller();
        u8 value();
        void write(u8 value);
        u8 read();
        void setButton(u32 buttonIndex, bool down);
    };

    extern Controller controller1;
    extern Controller controller2;
    void handle_event(const SDL_Event &event);
    u8 value();
}