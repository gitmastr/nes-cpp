#pragma once

#include "types.hpp"

const u64 SRAM_SIZE_BYTES = 0x2000;

class Cartridge
{
public:
    Cartridge(const string filename);
    ~Cartridge();

    u8 readPRG(u16 addr);
    u8 readCHR(u16 addr);
    u8 readSRAM(u16 addr);
    void writePRG(u16 addr, u8 value);
    void writeCHR(u16 addr, u8 value);
    void writeSRAM(u16 addr, u8 value);
    u8 getMapper();
    u8 getMirror();
    u8 getBattery();
    u64 getPRGSize();
    u64 getCHRSize();
    u64 getSRAMSize();
private:
    void init(vector<u8> &prg, vector<u8> &chr, u8 mapper, u8 mirror, u8 battery);
    vector<u8> prg;
    vector<u8> chr;
    vector<u8> sram;
    u8 mapper;
    u8 mirror;
    u8 battery;
};