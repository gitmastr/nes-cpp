#include <iostream>
#include <iomanip>
#include "console.hpp"
#include "cpu.hpp"
#include "mapper.hpp"
#include "SDL2/SDL.h"

int main(int argc, char *argv[])
{
    Console::init("roms/nestest.nes");

    for (auto i = 0; i < 10000000; i++)
        Console::step();

    std::cout << "Cycles: " << std::setprecision(2) << (CPU::cycles / 1000000000.) << " billion" << std::endl;

    Console::deinit();
    return 0;
}