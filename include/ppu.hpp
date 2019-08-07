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
        extern array<u8, 2048> data;
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
}