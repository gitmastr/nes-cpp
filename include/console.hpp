#pragma once

#include <memory>
#include "types.hpp"

class Mapper;

namespace Console
{
    extern vector<u8> ram;
    extern std::unique_ptr<Mapper> mapper;

    bool init(const string& fileName);
}