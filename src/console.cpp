#include <memory>
#include <cstdio>
#include "console.hpp"
#include "ppu.hpp"
#include "mapper.hpp"
#include "cartridge.hpp"
#include "cpu.hpp"


namespace Console
{
    array<u8, CONSOLE_RAM_BYTES> ram;
    std::unique_ptr<Mapper> mapper;

    bool init(const string& fileName)
    {
        Cartridge::init(fileName);
        mapper = std::move(Mapper::generateMapper());
        CPU::init();
        PPU::init();
        return true;
    }

    void deinit()
    {
    }

    u32 step()
    {
        CPU::printInstruction();
        u32 cpu_cycles = CPU::step();
        u32 to_run_ppu = 3 * cpu_cycles;

        for (u32 i = 0; i < to_run_ppu; i++)
        {
            PPU::tick();
        }

        return cpu_cycles;
    }
}