#include "ppu.hpp"
#include "cpu.hpp"
#include "cartridge.hpp"
#include "console.hpp"
#include "mapper.hpp"
#include "display.hpp"
#include "palettedata.hpp"
#include "ppuutils.hpp"
#include <exception>
#include <cassert>
#include <algorithm>

const u32 SCREEN_WIDTH = 256;
const u32 SCREEN_HEIGHT = 240;
const double SCREEN_REFRESH_RATE = 60.0988;
const u32 NUM_DOTS = 341;
const u32 NUM_SCAN_LINES = 262;
const u32 OAM_SIZE = 256;
const u32 SEC_OAM_SIZE = 32;
const u32 PALETTE_SIZE = 32;
const u32 TILE_WIDTH = 8;
const u32 TILE_HEIGHT = 8;

const bool VERBOSE = false;

namespace PPUMemory
{
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
        address = address % 0x4000;
        if (address < 0x2000) return Console::mapper->read(address);
        if (address < 0x3F00) return PPU::Nametable::read(mirror_address(address) % 2048);
        if (address < 0x4000) return PPU::Palette::read(address % 32);
        throw std::invalid_argument("invalid address in PPU memory read");
    }

    void write(u16 address, u8 value)
    {
        address = address % 0x4000;
        if (address < 0x2000) Console::mapper->write(address, value);
        else if (address < 0x3F00)
                              PPU::Nametable::write(mirror_address(address) % 2048, value);
        else if (address < 0x4000)
                              PPU::Palette::write(address % 32, value);
        else throw std::invalid_argument("invalid address in PPU memory write");
    }
}

namespace PPU 
{
    using PPUUtils::get_bit;

    enum class Register
    {
        PPUCTRL   = 0x2000,
        PPUMASK   = 0x2001,
        PPUSTATUS = 0x2002,
        OAMADDR   = 0x2003,
        OAMDATA   = 0x2004,
        PPUSCROLL = 0x2005,
        PPUADDR   = 0x2006,
        PPUDATA   = 0x2007,
        OAMDMA    = 0x4014
    };

    bool latch;
    u32 scan_line; // ranges from 0 - NUM_DOTS
    u32 dot; // ranges from 0 - NUM_DOTS
    u64 frame_count; // number of frames outputted

    namespace ADDR
    {
        u16 temp_vram_address;
        u16 vram_address;
        u8 fine_x_scroll;
        u8 coarse_x_scroll() { return vram_address & 0b11111; }
        u8 coarse_y_scroll() { return (vram_address >> 5) & 0b11111; }
        bool h_nametable() { return (vram_address >> 10) & 1; }
        bool v_nametable() { return (vram_address >> 11) & 1; }
        u8 fine_y_scroll() { return (vram_address >> 12) & 0b111; }

        void write(u8 value)
        {
            if (!latch) // first write
            {
                if (VERBOSE) printf("[ADDR] Wrote value %X on first pass\n", value);
                temp_vram_address &= 0b11111111;
                temp_vram_address |= (value & 0b111111) << 8;
            }
            else // second write
            {
                if (VERBOSE) printf("[ADDR] Wrote value %X on second pass\n", value);
                temp_vram_address &= 0b1111111100000000;
                temp_vram_address |= value;
                vram_address = temp_vram_address;
            }

            latch = !latch;
        }
    }

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
        bool V; // generate an NMI on the next vblank

        void write(u8 value)
        {
            if (VERBOSE) printf("[CTRL] Wrote value 0x%X to PPUCTRL\n", value);

            NN = value & 0b00000011;
            I  = (value & 0b00000100) > 0;
            S  = (value & 0b00001000) > 0;
            B  = (value & 0b00010000) > 0;
            H  = (value & 0b00100000) > 0;
            P  = (value & 0b01000000) > 0;
            V  = (value & 0b10000000) > 0;

            // t: ....BA.. ........ = d: ......BA
            ADDR::temp_vram_address |= 0b0000110000000000;
            ADDR::temp_vram_address &= NN * 0b10000000000;
        }

        u8 read()
        {
            u8 res = NN
                 | (I << 2)
                 | (S << 3)
                 | (B << 4)
                 | (H << 5)
                 | (P << 6)
                 | (V << 7);

            if (VERBOSE) printf("[CTRL] Read value 0x%X from PPUCTRL\n", res);

            return res;
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

            if (VERBOSE) printf("[MASK] Wrote value 0x%X to PPUMASK\n", value);
        }

        u8 read()
        {
            u8 res = (G << 0)
                 | (m << 1)
                 | (M << 2)
                 | (b << 3)
                 | (s << 4)
                 | (emph_R << 5)
                 | (emph_G << 6)
                 | (emph_B << 7);

            if (VERBOSE) printf("[MASK] Read value 0x%X from PPUMASK\n", res);

            return res;
        }
    }

    namespace STATUS // $2002
    {
        u8 reserved = 0;
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
            u8 res = (reserved & 0b11111)
                 | (O << 5)
                 | (S << 6)
                 | (V << 7);

            latch = false;
            V = false;
            
            if (VERBOSE) printf("[STATUS] Read value 0x%X from PPUSTATUS\n", res);
            return res;
        }
    }

    namespace SCROLL 
    {
        void write(u8 value)
        {
            if (!latch) // first write
            {
                // t: ....... ...HGFED = d: HGFED...
                // x:              CBA = d: .....CBA
                // w:                  = 1
                ADDR::temp_vram_address |= 0b11111;
                ADDR::temp_vram_address &= value >> 3;
                ADDR::fine_x_scroll = value & 0b111;
            }
            else // second write
            {
                // t: CBA..HG FED..... = d: HGFEDCBA
                // w:                  = 0
                
                                          // CBA..HGFED.....
                ADDR::temp_vram_address |= 0b111001111100000;
                                          //        HGFEDCBA
                ADDR::temp_vram_address &= (value & 0b11111000) << 2;
                ADDR::temp_vram_address &= (value & 0b111) << 12;
            }



            latch = !latch;
            if (VERBOSE) printf("[SCROLL] Wrote value 0x%X to PPUSCROLL\n", value);
        }
    }

    namespace DATA
    {
        u8 buffer = 0; // for reads of $2007 PPUDATA

        u8 read()
        {
            u8 res = PPUMemory::read(ADDR::vram_address);
            if (ADDR::vram_address % 0x4000 < 0x3F00)
            {
                std::swap(res, buffer);
            }
            else 
            {
                buffer = PPUMemory::read(ADDR::vram_address - 0x1000);
            }

            ADDR::vram_address += (CTRL::I) ? 32 : 1;
            if (VERBOSE) printf("[DATA] Read value 0x%X to PPUDATA\n", res);
            return res;
        }

        void write(u8 value)
        {
            PPUMemory::write(ADDR::vram_address, value);
            ADDR::vram_address += (CTRL::I) ? 32 : 1;
            if (VERBOSE) printf("[DATA] Wrote value 0x%X to PPUDATA\n", value);

        }
    }

    namespace Nametable 
    {
        array<u8, 2048> data;

        void write(u16 address, u8 value)
        {
            data[address] = value;
            if (VERBOSE) printf("[Nametable] Wrote value 0x%X to address 0x%X\n", value, address);
        }

        u8 read(u16 address)
        {
            u8 res = data[address];
            if (VERBOSE) printf("[Nametable] Read value 0x%X from nametable\n", res);
            return res;
        }
    }

    namespace Palette 
    {
        array<u8, 32> data { 0x09,0x01,0x00,0x01,
                             0x00,0x02,0x02,0x0D,
                             0x08,0x10,0x08,0x24,
                             0x00,0x00,0x04,0x2C,
                             0x09,0x01,0x34,0x03,
                             0x00,0x04,0x00,0x14,
                             0x08,0x3A,0x00,0x02,
                             0x00,0x20,0x2C,0x08 };

        u8 get_shift(u8 row, u8 col)
        {

            // bottom right, top right, bottom left, top left
            const u8 shifts[] = { 6, 2, 4, 0 };
            //                left               top
            return shifts[(col % 4 < 2) << 1 | (row % 4 < 2)];
        }

        u16 mirror(u16 address)
        {
            // Addresses $3F10/$3F14/$3F18/$3F1C are 
            // mirrors of $3F00/$3F04/$3F08/$3F0C.
            return (address >= 16 && address % 4 == 0) ?
                address - 16 :
                address;
        }

        u8 read(u16 address)
        {
            u8 res = data[mirror(address)];
            if (VERBOSE) printf("[PALETTE] Read value 0x%X from palette\n", res);

            return res;
        }

        void write(u16 address, u8 value)
        {
            data[mirror(address)] = value;
            if (VERBOSE) printf("[PALETTE] Wrote value 0x%X to palette\n", value);
        }

        u32 read_rgb(u16 address)
        {
            return PaletteData::data[read(address)];
        }
    }

    namespace OAM 
    {
        vector<u8> data(256);
        vector<u8> secondary_data(32);
        vector<u8> shift_reg_1(8);
        vector<u8> shift_reg_2(8);
        vector<u8> latches(8);
        vector<u8> counters(8);
        u8 address;

        struct SpritePixel
        {
            u32 rgb;
            bool is_opaque;
            bool sprite_behind_bg;
            u8 sprite_index;

            SpritePixel(u32 rgb, bool is_opaque, bool sprite_behind_bg, u8 sprite_index) :
                rgb(rgb), is_opaque(is_opaque), sprite_behind_bg(sprite_behind_bg),
                sprite_index(sprite_index)
            {
            }
        };

        SpritePixel evaluate_row_pixel(u32 cx)
        {
            for (u8 i = 0; i < 8; i++)
            {
                u32 x = counters[i];
                u8 attrib = latches[i];
                u8 palette = attrib & 0b11;
                bool flip_horiz = get_bit(attrib, 6);
                bool flip_vert = get_bit(attrib, 7);
                (void) flip_vert;
                bool priority = get_bit(attrib, 5);
                u8 bit_index = flip_horiz ? cx - x : 7 + x - cx;
                if (x <= cx && cx < x + 8) // hit
                {
                    u8 pixel = (2 * get_bit(shift_reg_2[i], bit_index))  | 
                                    get_bit(shift_reg_1[i], bit_index);
                    // if (cx == 255) printf("Hit pixel on last column\n");
                    if (pixel > 0)
                    {
                        // u32 rgb = 0;
                        // switch (pixel)
                        // {
                        //     case 1: rgb = 0x333333; break;
                        //     case 2: rgb = 0x888888; break;
                        //     case 3: rgb = 0XAAAAAA; break;
                        // }
                        return SpritePixel(Palette::read_rgb(4 * (palette + 4) + pixel), true, priority, i);
                    }
                }
            }

            return SpritePixel(0, false, false, 0);
        }

        u8 read_address()
        {
            u8 res = address;
            if (VERBOSE) printf("[OAM] Read value 0x%X from OAMADDR\n", res);
            return res;
        }

        u8 read_data()
        {
            u8 res = data[address];
            if (VERBOSE) printf("[OAM] Read value 0x%X from OAMDATA\n", res);
            return res;
        }

        void write_address(u8 _address)
        {
            address = _address;
            if (VERBOSE) printf("[OAM] Wrote value 0x%X to OAMADDR\n", _address);
        }

        void write_data(u8 value)
        {
            data[address++] = value;
            // TODO: should not increment
            //       during vblank
            if (VERBOSE) printf("[OAM] Wrote value 0x%X to OAMDATA\n", value);

        }

        void dma(u8 value)
        {
            u16 addr = value << 8;
            for (u32 i = 0; i < 0x100; i++)
                data[address++] = CPUMemory::read(addr++);

            CPU::stall += 513;

            if (CPU::cycles % 2 == 1)
                CPU::stall += 1;
            if (VERBOSE) printf("[OAM] OAM DMA complete\n");

        }

        void print_secondary_oam()
        {
            for (auto dat = secondary_data.begin(); dat < secondary_data.end(); dat += 4)
            {
                auto Y = *dat;
                auto index = *(dat + 1);
                auto attrib = *(dat + 2);
                auto X = *(dat + 3);
                printf("%ld. (%d, %d, %02X, %d)\n", 
                    (dat - secondary_data.begin()) / 4, Y, index, attrib, X);
            }
        }
    }

    void evaluate_sprites()
    {
        assert(dot == 257);

        std::fill(OAM::secondary_data.begin(), OAM::secondary_data.end(), 0xFF);

        u32 sec_idx = 0;
        u32 m = 0;
        for (u32 n = 0; n < 64; n++)
        {
            if (sec_idx < 8)
            {
                u32 y = OAM::data[4 * n];
                if (y <= scan_line && scan_line < y + 8) // sprite hit
                {
                    for (u32 c = 0; c < 4; c++) OAM::secondary_data[4 * sec_idx + c] = OAM::data[4 * n + c];
                    sec_idx++;
                }
            }
            else
            {
                u32 y = OAM::data[4 * n + m];
                if (y <= scan_line && scan_line < y + 8) // if in range
                {
                    STATUS::O = true; // set sprite overflow
                    m = 0;
                }
                else
                {
                    m = (m + 1) % 4;
                }
            }

        }

        for (u32 i = 0; i < 8; i++)
        {
            if (i < sec_idx)
            {
                u8 x = OAM::secondary_data[4 * i + 3];
                u8 y = OAM::secondary_data[4 * i];
                OAM::latches[i] = OAM::secondary_data[4 * i + 2];
                OAM::counters[i] = x;
                u16 lower_addr = (CTRL::S ? 0x1000 : 0) + 16 * OAM::secondary_data[4 * i + 1] + (scan_line - y);
                OAM::shift_reg_1[i] = PPUMemory::read(lower_addr);
                OAM::shift_reg_2[i] = PPUMemory::read(lower_addr + 8);
            }
            else 
            {
                OAM::latches[i] = 0xFF;
                OAM::counters[i] = 0xFF;
                OAM::shift_reg_1[i] = 0x00;
                OAM::shift_reg_2[i] = 0x00;
            }
        }
        // OAM::print_secondary_oam();
    }

    void vertical_blank()
    {
        // for (u8 row = 0; row < 30; row++)
        // {
        //     for (u8 col = 0; col < 32; col++)
        //     {
        //         u16 byte = col + row * 32;
        //         u8 fetched = PPUMemory::read(0x2000 + byte);
        //         u16 offset = CTRL::B ? 0x1000 : 0x0;
                
        //         u16 attrib_idx = (col / 4) + (row / 4) * 8;
        //         u8 attrib = PPUMemory::read(0x23C0 + attrib_idx);

        //         u8 palette = (attrib >> Palette::get_shift(row, col)) & 0b11;

        //         for (u8 drow = 0; drow < 8; drow++)
        //         {
        //             u8 first = PPUMemory::read(offset + 16 * fetched + drow);
        //             u8 second = PPUMemory::read(offset + 16 * fetched + drow + 8);
        //             for (u8 z = 0; z < 8; z++)
        //             {
        //                 u8 pixel = 2 * ((second >> z) & 1) + ((first >> z) & 1);
        //                 u32 rgbcolor;
        //                 if (pixel > 0) rgbcolor = PaletteData::data[Palette::read(
        //                             4 * palette + pixel
        //                         )];
        //                 else rgbcolor = PaletteData::data[Palette::read(0)];

        //                 Display::writePixel(col * 8 + 7 - z, row * 8 + drow, rgbcolor);
        //             }

        //         }
        //     }
        // }

        // for (u32 oam_idx = 64 - 1; oam_idx--;)
        // {
        //     u8 y_pos = OAM::data[4 * oam_idx + 0];
        //     u8 index_no = OAM::data[4 * oam_idx + 1] % 0x100;
        //     u8 attrib = OAM::data[4 * oam_idx + 2];
        //     u8 x_pos = OAM::data[4 * oam_idx + 3];

        //     u8 palette = attrib & 0b11;
        //     bool behind_bg = get_bit(attrib, 5);
        //     bool flip_horiz = get_bit(attrib, 6);
        //     bool flip_vert = get_bit(attrib, 7);

            
        //     u16 pattern_tab_idx = (CTRL::S ? 0x1000 : 0x0) + 16 * index_no;

        //     for (u32 row = 0; row < 8; row++)
        //     {
        //         u8 first = PPUMemory::read(pattern_tab_idx + row);
        //         u8 second = PPUMemory::read(pattern_tab_idx + row + 8);

        //         for (u32 col = 0; col < 8; col++)
        //         {
        //             u32 pixel_row = y_pos + (flip_vert ? 7 - row : row);
        //             u32 pixel_col = x_pos + (flip_horiz ? col : 7 - col);
        //             if (pixel_row < 240 && pixel_col < 256)
        //             {
        //                 u8 pixel = 2 * get_bit(second, col) + get_bit(first, col);
        //                 if (pixel > 0)
        //                 {
        //                     u32 rgbcolor = PaletteData::data[
        //                         Palette::read(4 * (palette + 4) + pixel)
        //                     ];
        //                     // printf("Writing to pixel (%d, %d)...\n", pixel_col, pixel_row);
        //                     Display::writePixel(pixel_col, pixel_row, rgbcolor);
        //                 }
        //             }
        //         }
        //     }
        // }

        Display::flip();


        if (CTRL::V)
        {
            CPU::trigger_nmi();
        }
    }

    void draw_row()
    {
        u8 row = scan_line;
        assert(row < 240);

        for (u32 col = 0; col < 256; col++)
        {
            u16 nametable_index = col / 8 + (row / 8) * 32;
            u8 fetched = PPUMemory::read(0x2000 + nametable_index);
            u16 bg_offset = CTRL::B ? 0x1000 : 0;
            u8 first  = PPUUtils::reverse_byte(PPUMemory::read(bg_offset + 16 * fetched + (row % 8)));
            u8 second = PPUUtils::reverse_byte(PPUMemory::read(bg_offset + 16 * fetched + (row % 8) + 8));
            u8 pixel = 2 * get_bit(second, col % 8) + get_bit(first, col % 8);
            u16 attrib_idx = (col / 32) + (row / 32) * 8;
            u8 attrib = PPUMemory::read(0x23C0 + attrib_idx);
            u8 palette = (attrib >> Palette::get_shift(row / 8, col / 8)) & 0b11;

            u32 rgb_bg;
            bool bg_opaque = pixel > 0;
            if (bg_opaque)
                rgb_bg = PaletteData::data[Palette::read(4 * palette + pixel)];
            else
                rgb_bg = PaletteData::data[Palette::read(0)];
            
            OAM::SpritePixel sp = OAM::evaluate_row_pixel(col);
            bool sprite_opaque = sp.is_opaque;
            u32 rgb_sprite = sp.rgb;
            bool sprite_behind_bg = sp.sprite_behind_bg;

            switch ((bg_opaque << 2) | (sprite_opaque << 1) | sprite_behind_bg)
            {
                case 0b000: Display::writePixel(col, row, rgb_bg); break;
                case 0b001: Display::writePixel(col, row, rgb_bg); break;
                case 0b010: Display::writePixel(col, row, rgb_sprite); break;
                case 0b011: Display::writePixel(col, row, rgb_sprite); break;
                case 0b100: Display::writePixel(col, row, rgb_bg); break;
                case 0b101: Display::writePixel(col, row, rgb_bg); break;
                case 0b110: Display::writePixel(col, row, rgb_sprite); break;
                case 0b111: Display::writePixel(col, row, rgb_bg); break;
            }
        }
    }

    void tick()
    {
        // if (scan_line % 5 == 0 && dot == 0)
        //     printf("Dot: %d, Scanline: %d\n", dot, scan_line);
        dot++;
        if (dot == NUM_DOTS)
        {
            dot = 0;
            scan_line++;
        }
        if (scan_line == NUM_SCAN_LINES)
        {
            scan_line = 0;
            frame_count++;
        }

        /* Rendering logic goes here */

        if (scan_line == NUM_SCAN_LINES - 1 && dot == 1)
        {
            STATUS::V = false;
        }
        else if (scan_line == 241 && dot == 1)
        {
            STATUS::V = true;
            vertical_blank();
        }

        if (dot == 257 && scan_line < 240)
        {
            draw_row();
            evaluate_sprites();
        }
    }

    u8 read_register(u16 address)
    {
        if (VERBOSE) printf("[PPUREG] Reading register 0x%X\n", address);
        switch (static_cast<Register>(address))
        {
            case Register::PPUCTRL:   return CTRL::read();        // 0x2000
            case Register::PPUMASK:   return MASK::read();        // 0x2001
            case Register::PPUSTATUS: return STATUS::read();      // 0x2002 (?)
            case Register::OAMADDR:   return OAM::read_address();
            case Register::OAMDATA:   return OAM::read_data();
            case Register::PPUDATA:   return DATA::read();
            default: 
                throw std::invalid_argument("unknown register in read");
        }
    }

    void write_register(u16 address, u8 value)
    {
        if (VERBOSE) printf("Writing to register 0x%X\n", address);
        switch (static_cast<Register>(address))
        {
            case Register::PPUCTRL: 
                CTRL::write(value); 
                break;
            case Register::PPUMASK:
                MASK::write(value); 
                break;
            case Register::OAMADDR: 
                OAM::write_address(value); 
                break;
            case Register::OAMDATA: 
                OAM::write_data(value);
                break;
            case Register::PPUSCROLL: 
                SCROLL::write(value); 
                break;
            case Register::PPUADDR: 
                ADDR::write(value); 
                break;
            case Register::PPUDATA:
                DATA::write(value);
                break;
            case Register::OAMDMA:
                OAM::dma(value);
                break;
            default:
                throw std::invalid_argument("unknown register in write");
        }
    }

    void init()
    {
        latch = false;
        scan_line = 240;
        dot = 340;
        frame_count = 0;
        CTRL::write(0);
        MASK::write(0);
        OAM::write_address(0);
    }
}