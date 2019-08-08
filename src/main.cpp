#include <iostream>
#include <iomanip>
#include "console.hpp"
#include "cpu.hpp"
#include "mapper.hpp"
#include "SDL2/SDL.h"

const u64 CPU_CYCLES_MAX = static_cast<u64>(1) << 50;

int main(int argc, char *argv[])
{
    if (argc < 2) 
    {
        printf("\n\tUsage: %s <romname>.nes\n", argv[0]);
        exit(1);
    }

    Console::init(argv[1]);
    Console::run();
    Console::deinit();
    return 0;
}