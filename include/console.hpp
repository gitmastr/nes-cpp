#pragma once

#include "types.hpp"
#include "cartridge.hpp"
#include "cpu.hpp"

class Console
{
public:
    Console(const string romName);
    ~Console();

    string &getRom();
private:
    Cartridge cartridge;
    CPU cpu;
};