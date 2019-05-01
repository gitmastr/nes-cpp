#pragma once

#include "types.hpp"
#include "cartridge.hpp"

class Mapper
{
public:
    virtual u8 read(u16 addr) = 0;
    virtual void write(u16 addr, u8 value) = 0;
    virtual void step() = 0;
};

class Mapper2 : private Mapper
{
public:
    Mapper2(Cartridge &cartridge);
    ~Mapper2();
private:
    u8 read(u16 addr);
    void write(u16 addr, u8 value);
    void step();


    Cartridge &cartridge;
    u32 prgBanks;
    u32 prgBank1;
    u32 prgBank2;
};