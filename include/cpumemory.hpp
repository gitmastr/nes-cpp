#pragma once

class CPUMemory;

#include "types.hpp"

class Console;

class CPUMemory
{
public:
    CPUMemory(Console &console);
    ~CPUMemory();
    u8 read(u16 addr);
    void write(u16 addr, u8 value);
private:
    Console &console;
};