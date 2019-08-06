#include <memory>
#include "console.hpp"
#include "mapper.hpp"
#include "cartridge.hpp"
#include "cpu.hpp"
#include "ppu.hpp"
#include "display.hpp"

const u64 CONSOLE_RAM_BYTES = 2048;

namespace Console
{
    vector<u8> ram(CONSOLE_RAM_BYTES);
    std::unique_ptr<Mapper> mapper;

    bool init(const string& fileName)
    {
        Cartridge::init(fileName);
        mapper = std::move(Mapper::generateMapper());
        CPU::init();
        PPU::init();
        Display::init(5);
        return true;
    }

    void deinit()
    {
        Display::deinit();
    }

    u32 step()
    {
        u32 cpu_cycles = CPU::step();
        u32 ppu_cycles = cpu_cycles * 3;
        for (u32 i = 0; i < ppu_cycles; i++)
        {
            PPU::step();
            mapper->step();
        }

        // APU step

        return cpu_cycles;
    }
}