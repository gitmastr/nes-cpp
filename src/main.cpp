#include <iostream>
#include <iomanip>
#include "console.hpp"
#include "cpu.hpp"
#include "mapper.hpp"
#include "SDL2/SDL.h"

const u64 CPU_CYCLES_MAX = 60000;

int main(int argc, char *argv[])
{
    Console::init("roms/color_test.nes");

    u64 cycles = 0;
    while (cycles < CPU_CYCLES_MAX)
    {
        cycles += Console::step();
    }

    Console::deinit();
    return 0;
}