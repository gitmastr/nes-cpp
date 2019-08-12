#include "ppuutils.hpp"

namespace PPUUtils
{
    bool get_bit(u8 value, u32 bit)
    {
        return (value >> bit) & 1;
    }

    u8 reverse_byte(u8 v)
    {
        v = (v << 4) | (v >> 4);
        v = ((v & 0b00110011) << 2) | ((v & 0b11001100) >> 2);
        v = ((v & 0b01010101) << 1) | ((v & 0b10101010) >> 1);
        return v;
    }
}