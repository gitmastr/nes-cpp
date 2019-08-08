#pragma once

#include "types.hpp"

namespace PPU
{
    extern u64 frame_count;

    namespace CTRL
    {
        void write(u8 value);
        u8 read();
    }

    namespace MASK
    {
        void write(u8 value);
        u8 read();
    }

    namespace STATUS
    {
        void write(u8 value);
        u8 read();
    }

    namespace ADDR
    {
        extern u16 vram_address;
        extern u16 temp_vram_address;
        void write(u8 value);
    }

    namespace Nametable
    {
        extern array<u8, 2048> data;
        void write(u16 address, u8 value);
        u8 read(u16 address);
    }

    namespace OAM
    {
        extern array<u8, 256> data;
    }

    namespace Palette
    {
        extern array<u8, 32> palette;
        u8 read(u16 address);
        void write(u16 address, u8 value);
    }

    void init();
    void tick();
    u8 read_register(u16 address);
    void write_register(u16 address, u8 value);
}