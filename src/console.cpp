#include "console.hpp"

Console::Console(const string romName) : cartridge(romName), cpu(*this)
{
    
}


Console::~Console()
{

}