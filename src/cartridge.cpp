#include <iterator>
#include <fstream>
#include "cartridge.hpp"
#include <cstdio>

Cartridge::Cartridge(const string filename) : sram(0x2000)
{
    std::ifstream file(filename, std::ios::binary);
    file.unsetf(std::ios::skipws);

    std::streampos fileSize;

    file.seekg(0, std::ios::end);
    fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    vector<u8> buf(fileSize);
    buf.insert(buf.begin(),
               std::istream_iterator<u8>(file),
               std::istream_iterator<u8>());

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
    u8 mapper = mapper1 | (mapper2 << 4);

    u8 trainer = (control1 & 4) == 4;
    (void) trainer;

    u8 mirror1 = control1 & 1;
    u8 mirror2 = (control2 >> 3) & 1;
    u8 mirror = mirror1 | (mirror2 << 1);

    u8 battery = (control1 >> 1) & 1;

    size_t prgSize = 16384 * numPRG;
    size_t chrSize = 8192 * std::max<u64>(numCHR, 1);

    size_t loc = (trainer ? 528 : 16);

    vector<u8> prg(buf.begin() + loc, buf.begin() + loc + prgSize);
    vector<u8> chr(buf.begin() + loc + prgSize, buf.begin() + loc + prgSize + chrSize);

    this->init(prg, chr, mapper, mirror, battery);
}

u8 Cartridge::readPRG(u16 addr) { return prg[addr]; }
u8 Cartridge::readCHR(u16 addr) { return chr[addr]; }
u8 Cartridge::readSRAM(u16 addr) { return sram[addr]; }
void Cartridge::writePRG(u16 addr, u8 value) { prg[addr] = value; }
void Cartridge::writeCHR(u16 addr, u8 value) { chr[addr] = value; }
void Cartridge::writeSRAM(u16 addr, u8 value) { sram[addr] = value; }
u8 Cartridge::getMapper() { return mapper; }
u8 Cartridge::getMirror() { return mirror; }
u8 Cartridge::getBattery() { return battery; }
u64 Cartridge::getPRGSize() { return prg.size(); }
u64 Cartridge::getCHRSize() { return chr.size(); }
u64 Cartridge::getSRAMSize() { return sram.size(); }

void Cartridge::init(vector<u8> &prg, vector<u8> &chr, u8 mapper, u8 mirror, u8 battery)
{
    this->prg = prg;
    this->chr = chr;
    this->mapper = mapper;
    this->mirror = mirror;
    this->battery = battery;
}

Cartridge::~Cartridge()
{

}