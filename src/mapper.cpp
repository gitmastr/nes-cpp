#include <memory>
#include <cassert>
#include "mapper.hpp"
#include "console.hpp"
#include "cartridge.hpp"

Mapper::Mapper()
{

}

Mapper::~Mapper()
{

}

std::unique_ptr<Mapper> Mapper::generateMapper()
{
    assert(Cartridge::mapper == 0);
    switch (Cartridge::mapper)
    {
        case 0: return std::make_unique<Mapper2>();
        case 2: return std::make_unique<Mapper2>();
        default: throw "unknown mapper";
    }
    return std::make_unique<Mapper2>();
}

Mapper2::Mapper2()
{
    prgBanks = Cartridge::prg.size() / 0x4000;
    prgBank1 = 0;
    prgBank2 = prgBanks - 1;
}

Mapper2::~Mapper2()
{
}

u8 Mapper2::read(u16 addr)
{
    if (addr  < 0x2000) return Cartridge::chr[addr];
    if (addr >= 0xC000) return Cartridge::prg[(prgBank2 * 0x4000 + addr) - 0xC000];
    if (addr >= 0x8000) return Cartridge::prg[(prgBank1 * 0x4000 + addr) - 0x8000];
    if (addr >= 0x6000) return Cartridge::sram[addr - 0x6000];

    throw "Invalid Mapper2 read";
    return 0;
}

void Mapper2::write(u16 addr, u8 value)
{
    if      (addr  < 0x2000) Cartridge::chr[addr] = value;
    else if (addr >= 0x8000) prgBank1 = value % prgBanks;
    else if (addr >= 0x6000) Cartridge::sram[addr - 0x6000] = value;
    else throw "Invalid Mapper2 write";
}

void Mapper2::step()
{

}