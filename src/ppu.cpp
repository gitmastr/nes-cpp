#include "ppu.hpp"

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


namespace PPU 
{
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
    }

    namespace VRAM
    {
        u16 vram_address;
        u8 coarse_x_scroll() { return vram_address & 0b11111; }
        u8 coarse_y_scroll() { return (vram_address >> 5) & 0b11111; }
        bool h_nametable() { return (vram_address >> 10) & 1; }
        bool v_nametable() { return (vram_address >> 11) & 1; }
        u8 fine_y_scroll() { return (vram_address >> 12) & 0b111; }
    }

    

}