#include "input.hpp"

namespace Input 
{
    namespace Controller
    {
        namespace Buttons
        {
            enum { A = 0, B, select, start, up, down, left, right };
        }
        vector<bool> buttons(8);
        u8 index = 0; 
        bool polling = false;

        u8 read()
        {
            return buttons[polling ? (index = 0) : (index++)];
        }

        void write(u8 value)
        {
            polling = value & 1;
            index = 0;
        }
    }


    void handle_event(const SDL_Event &event)
    {
        bool down = event.type == SDL_KEYDOWN;
        if (!down && event.type != SDL_KEYUP)
            throw std::invalid_argument("unknown event type");

        SDL_Keycode key_pressed = event.key.keysym.sym;

        const array<SDL_Keycode, 8> default_keys = {{
            SDLK_a,
            SDLK_b,
            SDLK_n,
            SDLK_m,
            SDLK_UP,
            SDLK_DOWN,
            SDLK_LEFT,
            SDLK_RIGHT
        }};

        for (u32 index = 0; index < 8; index++)
            if (key_pressed == default_keys[index]) 
            {
                Controller::buttons[index] = down;
                break;
            }

    }
}