#include <iterator>
#include <fstream>
#include <cstdio>
#include "cartridge.hpp"

namespace Cartridge
{
    const size_t SRAM_SIZE_BYTES = 0x2000;

    vector<u8> prg;
    vector<u8> chr;
    vector<u8> sram(SRAM_SIZE_BYTES);
    u8 mapper = 0;
    u8 mirror = 0;
    u8 battery = 0;

    void init(const string &fileName)
    {
        std::ifstream file(fileName, std::ios::in | std::ios::binary);
        file.unsetf(std::ios::skipws);

        file.seekg(0, std::ios::end);
        // auto fileSize = file.tellg();
        file.seekg(0, std::ios::beg);

        vector<u8> buf((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());

        if (buf[0] != 'N' ||
            buf[1] != 'E' ||
            buf[2] != 'S' ||
            buf[3] != 26)
            throw "Invalid iNES file";

        u8 numPRG = buf[4];
        u8 numCHR = buf[5];
        u8 control1 = buf[6];
        u8 control2 = buf[7];
        u8 numRAM = buf[8];
        (void) numRAM;

        u8 mapper1 = control1 >> 4;
        u8 mapper2 = control2 >> 4;
        mapper = mapper1 | (mapper2 << 4);

        u8 trainer = (control1 & 4) == 4;
        (void) trainer;

        u8 mirror1 = control1 & 1;
        u8 mirror2 = (control2 >> 3) & 1;
        mirror = mirror1 | (mirror2 << 1);

        battery = (control1 >> 1) & 1;

        size_t prgSize = 16384 * numPRG;
        size_t chrSize = 8192 * std::max<u64>(numCHR, 1);

        size_t loc = (trainer ? 528 : 16);

        prg = vector<u8>(buf.begin() + loc, buf.begin() + loc + prgSize);
        chr = vector<u8>(buf.begin() + loc + prgSize, buf.begin() + loc + prgSize + chrSize);
    }
}