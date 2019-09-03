#include "input.hpp"
#include <algorithm>

namespace Input 
{
    Controller controller1;
    Controller controller2;

    Controller::Controller() :
        buttons(8),
        index(0),
        polling(false)
    {
        std::fill(buttons.begin(), buttons.end(), false);
    }

    Controller::~Controller()
    {

    }

    u8 Controller::read()
    {
        return buttons[polling ? (index = 0) : (index++)];
    }

    void Controller::write(u8 value)
    {
        polling = value & 1;
        index = 0;
    }

    void Controller::setButton(u32 buttonIndex, bool down)
    {
        buttons[buttonIndex] = down;
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
        {
            if (key_pressed == default_keys[index]) 
            {
                controller1.setButton(index, down);
                break;
            }
        }
    }
}