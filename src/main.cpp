#include <iostream>
#include <SDL2/SDL.h>
#include "console.hpp"
#include "image.hpp"
#include "display.hpp"


int main(int argc, char *argv[])
{
    Console::init("roms/nestest.nes");
    Display::init(4);

    Image img(256, 240);

    img.fill(0, 0, 255);

    for (int x = 0; x < 240; x++)
    {
        img.setPixel(x, x, 0, 255, 0);
    }

    Display::loadImage(img);


    SDL_Delay(1000);


    SDL_Quit();
    return 0;
}