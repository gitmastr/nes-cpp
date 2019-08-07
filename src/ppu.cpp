#include "ppu.hpp"
#include "cartridge.hpp"
#include "console.hpp"
#include "mapper.hpp"
#include <exception>

const u32 SCREEN_WIDTH = 256;
const u32 SCREEN_HEIGHT = 240;
const double SCREEN_REFRESH_RATE = 60.0988;
const u32 NUM_DOTS = 341;
const u32 NUM_SCANLINESS = 262;
const u32 OAM_SIZE = 256;
const u32 SEC_OAM_SIZE = 32;
const u32 PALETTE_SIZE = 32;
const u32 TILE_WIDTH = 8;
const u32 TILE_HEIGHT = 8;
const u32 PATTERN_TABLE_0_START = 0x0000;
const u32 PATTERN_TABLE_1_START = 0x1000;
const u32 NAME_TABLE_START = 0x2000;
const u32 ATTRIBUTE_TABLE_START = 0x23C0;
const u32 BG_PALETTE_START = 0x3F00;
const u32 SPRITE_PALETTE_START = 0x3F10;
const u32 TILE_SIZE = 16;
const u32 NUM_PALETTE_ENTRIES = 4;
const u32 NUM_CHROMA_VALUES = 16;
const u32 NUM_LUMA_VALUES = 4;
const u32 NUM_SPRITES = 64;
const u32 NUM_SPRITES_PER_LINE = 8;

namespace PPUMemory
{
    using Cartridge::MirrorMode;

    const u8 mirror_table[][4] = 
        {
            { 0, 0, 1, 1 },
            { 0, 1, 0, 1 },
            { 0, 0, 0, 0 },
            { 1, 1, 1, 1 },
            { 0, 1, 2, 3 }
        };

    u16 mirror_address(u16 address)
    {
        address = address % 0x1000;
        u16 table = address / 0x0400;
        u16 offset = address % 0x0400;
        return 0x2000 + mirror_table[static_cast<int>(Cartridge::mirror_mode)][table] * 0x0400 + offset;
    }   

    u8 read(u16 address)
    {
        address = address & 0x4000;
        if (address < 0x2000) return Console::mapper->read(address);
        if (address < 0x3F00) return PPU::Nametable::data[mirror_address(address) % 2048];
        if (address < 0x4000) return PPU::Palette::read(address % 32);
        throw std::invalid_argument("invalid address");
    }
}

namespace PPU 
{
    bool latch = false;

    namespace CTRL
    {
        u8 NN; // base nametable address 
                 // (0 = $2000; 1 = $2400; 2 = $2800; 3 = $2C00)
                 // Equivalently, sets bits in scroll
                 // 7  bit  0
                 // ---- ----
                 // .... ..YX
                 //        ||
                 //        |+- 1: Add 256 to the X scroll position
                 //        +-- 1: Add 240 to the Y scroll position
        
        bool I; //  (0: add 1, going across; 1: add 32, going down)
        bool S; // sprite pattern table address for 8x8
                // (0: $0000; 1: $1000; ignored in 8x16 mode)
        bool B; // background pattern table address(0: $0000; 1: $1000)
        bool H; // sprite size (0: 8x8 pixels; 1: 8x16 pixels)
        bool P; // master/slave mode (0: read backdrop from EXT pins; 1: output color on EXT pins)
        bool V; // generate an NMI on the next vblack

        void write(u8 value)
        {
            NN = value & 0b00000011;
            I =  value & 0b00000100;
            S =  value & 0b00001000;
            B =  value & 0b00010000;
            H =  value & 0b00100000;
            P =  value & 0b01000000;
            V =  value & 0b10000000;
        }

        u8 read()
        {
            return NN
                 | (I << 2)
                 | (S << 3)
                 | (B << 4)
                 | (H << 5)
                 | (P << 6)
                 | (V << 7);
        }
    }

    namespace MASK
    {
        bool G; // grayscale
        bool m; // background show left column 
                // 1: Show background in leftmost 8 pixels of screen, 0: Hide
        bool M; // sprite show left column
                // 1: Show sprites in leftmost 8 pixels of screen, 0: Hide
        bool b; // show backgrounds
        bool s; // show sprites
        bool emph_R; // emphasize red
        bool emph_G; // emphasize green
        bool emph_B; // emphasize blue

        void write(u8 value)
        {
            G = value & 0b00000001;
            m = value & 0b00000010;
            M = value & 0b00000100;
            b = value & 0b00001000;
            s = value & 0b00010000;
            emph_R = value & 0b00100000;
            emph_G = value & 0b01000000;
            emph_B = value & 0b10000000;
        }

        u8 read()
        {
            return (G << 0)
                 | (m << 1)
                 | (M << 2)
                 | (b << 3)
                 | (s << 4)
                 | (emph_R << 5)
                 | (emph_G << 6)
                 | (emph_B << 7);
        }
    }

    namespace STATUS
    {
        u8 reserved;
        bool O; // sprite overflow. The intent was for this flag to be set
                // whenever more than eight sprites appear on a scanline, but a
                // hardware bug causes the actual behavior to be more complicated
                // and generate false positives as well as false negatives; see
                // PPU sprite evaluation. This flag is set during sprite
                // evaluation and cleared at dot 1 (the second dot) of the
                // pre-render line.
        bool S; // Sprite 0 hit. Set when a nonzero pixel of sprite
                // 0 overlaps a nonzero background pixel. Cleared at dot
                // 1 of the prender line.
        bool V; // Vertical blank has started.
                // Set at dot 1 of line 241 (the line *after* the post-render
                // line); cleared after reading $2002 and at dot 1 of the
                // pre-render line.

        u8 read()
        {
            return (reserved & 0b11111)
                 | (O << 5)
                 | (S << 6)
                 | (V << 7);
        }
    }

    namespace ADDR
    {
        u16 temp_vram_address;
        u16 vram_address;
        u8 coarse_x_scroll() { return vram_address & 0b11111; }
        u8 coarse_y_scroll() { return (vram_address >> 5) & 0b11111; }
        bool h_nametable() { return (vram_address >> 10) & 1; }
        bool v_nametable() { return (vram_address >> 11) & 1; }
        u8 fine_y_scroll() { return (vram_address >> 12) & 0b111; }

        void write(u8 value)
        {
            if (latch) // set high byte
                temp_vram_address = (vram_address & 0xFF) | ((value % 0x40) << 8);
            else       // set low byte
            {
                temp_vram_address = (temp_vram_address & 0xFF00) | value;
                vram_address = temp_vram_address;
            }

            latch = !latch;
        }
    }

    namespace Nametable 
    {
        vector<u8> data;
    }

    namespace Palette 
    {
        vector<u8> palette;
        u8 read(u16 address)
        {
            return 0;
        }
    }
    

}