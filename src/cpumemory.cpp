#include "types.hpp"
#include "cpumemory.hpp"
#include "console.hpp"

CPUMemory::CPUMemory(Console &console) : console(console)
{

}

CPUMemory::~CPUMemory()
{

}

u8 CPUMemory::read(u16 addr)
{
    return 0;
}

void CPUMemory::write(u16 addr, u8 value)
{
    
}