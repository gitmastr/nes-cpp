#pragma once

#include <memory>
#include "types.hpp"

class Mapper
{
protected:
    Mapper();
public:
    virtual ~Mapper() = 0;
    virtual u8 read(u16 addr) = 0;
    virtual void write(u16 addr, u8 value) = 0;
    virtual void step() = 0;

    static std::unique_ptr<Mapper> generateMapper();
};

class Mapper2 : public Mapper
{
public:
    Mapper2();
    ~Mapper2();
private:
    u8 read(u16 addr);
    void write(u16 addr, u8 value);
    void step();

    u32 prgBanks;
    u32 prgBank1;
    u32 prgBank2;
};