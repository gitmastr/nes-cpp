#pragma once

#include <memory>
#include "types.hpp"

class Mapper;

const u64 CONSOLE_RAM_BYTES = 2048;

namespace Console
{
    extern array<u8, CONSOLE_RAM_BYTES> ram;
    extern std::unique_ptr<Mapper> mapper;

    bool init(const string& fileName);
    void run();
    void deinit();
    u32 step();
}