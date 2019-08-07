#pragma once

#include "types.hpp"

namespace PPU
{
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
        void write(u8 value);
    }

    namespace Nametable
    {
        extern vector<u8> data;
    }

    namespace Palette
    {
        extern vector<u8> palette;
        u8 read(u16 address);
    }
}