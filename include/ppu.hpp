#pragma once

#include "types.hpp"

namespace PPUMemory
{
    u8 read(u16 addr);
    void write(u16 addr, u8 value);
}

class Image;
namespace PPU
{
    extern u32 cycle;
    extern u32 scanline;
    extern u64 frame;
    extern vector<u8> paletteData;
    extern vector<u8> nametableData;
    extern vector<u8> oamData;
    extern Image front;
    extern Image back;
    extern u16 v;
    extern u16 t;
    extern u8 x;
    extern bool latch;
    extern u8 f;
    extern u8 regis;
    extern bool nmiOccurred;
    extern bool nmiOutput;
    extern bool nmiPrevious;
    extern u8 nmiDelay;
    extern u8 nametableByte;
    extern u8 attributeTableByte;
    extern u8 lowTileByte;
    extern u64 tileData;
    extern u32 spriteCount;
    extern vector<u32> spritePatterns;
    extern vector<u8> spritePositions;
    extern vector<u8> spritePriorities;
    extern vector<u8> spriteIndices;
    extern u8 flagNametable;
    extern u8 flagIncrement;
    extern u8 flagSpriteTable;
    extern u8 flagBackgroundTable;
    extern u8 flagSpriteSize;
    extern u8 flagMasterSlave;
    extern u8 flagGrayscale;
    extern u8 flagShowLeftBackground;
    extern u8 flagShowLeftSprites;
    extern u8 flagShowBackground;
    extern u8 flagShowSprites;
    extern u8 flagRedTint;
    extern u8 flagGreenTint;
    extern u8 flagBlueTint;
    extern u8 flagSpriteZeroHit;
    extern u8 flagSpriteOverflow;
    extern u8 oamAddress;
    extern u8 bufferedData;

    u8 read_palette(u16 addr);
    void write_palette(u16 addr, u8 value);
    void init();
    void tick();
    void step();
    void nmi_change();
    void set_vertical_blank();
    void clear_vertical_blank();
    void swap_frame_buffer();
    void write_control(u8 value);
    void write_mask(u8 value);
    void write_oam_address(u8 value);
    void write_oam_data(u8 value);
    u8 read_oam_data();
    void evaluate_tile(u32 tlx, u32 tly, u32 pattern_idx);
    void write_register(u16 addr, u8 value);

    u16 nametable_mirror(u16 addr);
    u8 read_status();
}