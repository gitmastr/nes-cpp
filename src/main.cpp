#include <iostream>
#include <iomanip>
#include "console.hpp"
#include "image.hpp"
#include "display.hpp"
#include "cpu.hpp"
#include "ppu.hpp"
#include "mapper.hpp"
#include "SDL2/SDL.h"

int main(int argc, char *argv[])
{
    Console::init("roms/mario.nes");

    SDL_Event event;
    bool quit = false;

    for (int i = 0; i < 240; i++) PPU::nametableData[i] = i + 16;


    while (!quit)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT) quit = true;
        }

        for (u32 i = 0; i < (1 << 10); i++)
            Console::step();
    }

    std::cout << "Cycles: " << std::setprecision(2) << (CPU::cycles / 1000000000.) << " billion" << std::endl;
    std::cout << "Frames: " << PPU::frame << std::endl;

    Console::deinit();
    return 0;
}