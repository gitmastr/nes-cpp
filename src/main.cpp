#include <iostream>
#include <iomanip>
#include "console.hpp"
#include "cpu.hpp"
#include "mapper.hpp"
#include "SDL2/SDL.h"

const u64 CPU_CYCLES_MAX = static_cast<u64>(1) << 50;

int main(int argc, char *argv[])
{
    Console::init("roms/mrio.nes");
    Console::run();
    Console::deinit();
    return 0;
}