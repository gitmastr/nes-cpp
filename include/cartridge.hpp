#pragma once
#include "types.hpp"

const u64 SRAM_SIZE_BYTES = 0x2000;

namespace Cartridge
{
    enum class MirrorMode
    {
        Horizontal = 0,
        Vertical = 1,
        MirrorSingle0 = 2,
        MirrorSingle1 = 3,
        MirrorFour = 4
    };
    
    extern vector<u8> prg;
    extern vector<u8> chr;
    extern vector<u8> sram;
    extern u8 mapper;
    extern MirrorMode mirror_mode;
    extern u8 battery;

    void init(const string &fileName);
    void load(vector<u8> &prg, vector<u8> &chr, u8 mapper, u8 mirror, u8 battery);


}