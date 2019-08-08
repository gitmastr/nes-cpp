#include "input.hpp"

namespace Input 
{
    namespace Controller
    {
        bool up, down, left, right, start, select, A, B;
        u8 index = 0; 
        bool polling = false;

        u8 read()
        {
            u8 result = (value() >> index++) & 1;
            if (polling) index = 0;
            return result;
        }

        void write(u8 value)
        {
            switch (value)
            {
                case 0: 
                    polling = false; 
                    break;
                case 1:
                    polling = true;
                    index = 0;
                    break;
            }
        }

        u8 value()
        {
            return  A
                 | (B << 1)
                 | (select << 2)
                 | (start << 3)
                 | (up << 4)
                 | (down << 5)
                 | (left << 6)
                 | (right << 7);
        }
    }


    void handle_event(const SDL_Event &event)
    {
        bool down = event.type == SDL_KEYDOWN;
        if (!down && event.type != SDL_KEYUP)
            throw std::invalid_argument("unknown event type");

        auto key_pressed = event.key.keysym.sym;

        switch (key_pressed)
        {
            case SDLK_UP:    Controller::up     = down; break;
            case SDLK_DOWN:  Controller::down   = down; break;
            case SDLK_LEFT:  Controller::left   = down; break;
            case SDLK_RIGHT: Controller::right  = down; break;
            case SDLK_n:     Controller::start  = down; break;
            case SDLK_m:     Controller::select = down; break;
            case SDLK_a:     Controller::A      = down; break;
            case SDLK_b:     Controller::B      = down; break;
            default: return;
        }
    }
}