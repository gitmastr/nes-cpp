#include "mapper.hpp"

Mapper2::Mapper2(Cartridge &cartridge) : cartridge(cartridge) 
{
    prgBanks = cartridge.getPRGSize();
    prgBank1 = 0;
    prgBank2 = prgBanks - 1;
}

u8 Mapper2::read(u16 addr)
{
    u16 index;

    if (addr < 0x2000) return cartridge.readCHR(addr);
    if (addr >= 0xC000)
    {
        index = (prgBank2 * 0x4000 + addr) - 0xC000;
        return cartridge.readPRG(index);
    }
    if (addr >= 0x8000)
    {
        index = (prgBank1 * 0x4000 + addr) - 0x8000;
        return cartridge.readPRG(index);
    }
    if (addr >= 0x6000)
    {
        index = addr - 0x6000;
        return cartridge.readSRAM(index);
    }

    throw "Invalid Mapper2 read";
    return 0;
}

void Mapper2::write(u16 addr, u8 value)
{
    if (addr < 0x2000)
        cartridge.writeCHR(addr, value);
    else if (addr >= 0x8000)
        prgBank1 = value % prgBanks;
    else if (addr >= 0x6000)
        cartridge.writeSRAM(addr - 0x6000, value);
    else throw "Invalid Mapper2 write";
}

void Mapper2::step()
{

}