#include <memory>
#include "console.hpp"
#include "mapper.hpp"
#include "cartridge.hpp"
#include "cpu.hpp"

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
        return true;
    }

    void deinit()
    {
    }

    u32 step()
    {
        u32 cpu_cycles = CPU::step();

        return cpu_cycles;
    }
}