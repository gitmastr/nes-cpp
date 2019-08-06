#include "types.hpp"
#include "ppu.hpp"
#include "image.hpp"
#include "cartridge.hpp"
#include "mapper.hpp"
#include "console.hpp"
#include "display.hpp"
#include "cpu.hpp"
#include <cassert> 
#include <algorithm>

namespace PPUMemory
{
    u8 read(u16 addr)
    {
        addr = addr % 0x4000;

        if      (addr < 0x2000) return Console::mapper->read(addr);
        else if (addr < 0x3F00)
        {
            auto mode = Cartridge::mirror;
            (void) mode; // TODO: add other mirror modes
            return PPU::nametableData[PPU::nametable_mirror(addr) % 2048];
        }
        else if (addr < 0x4000) return PPU::read_palette(addr % 32);

        return 0;
    }

    void write(u16 addr, u8 value)
    {
        addr = addr % 0x4000;

        if      (addr < 0x2000) Console::mapper->write(addr, value);
        else if (addr < 0x3F00)
        {
            auto mode = Cartridge::mirror;
            (void) mode;
            u16 nametableDataAddr = PPU::nametable_mirror(addr); // TODO: add other mirror modes
            PPU::nametableData[nametableDataAddr % 2048] = value;
        }
        else if (addr < 0x4000)
        {
            PPU::write_palette(addr % 32, value);
        }
    }
}

namespace PPU
{
    using PPUMemory::read;
    using PPUMemory::write;

    u32 cycle = 0;
    u32 scanline = 0;
    u64 frame = 0;
    vector<u8> paletteData(32);
    vector<u8> nametableData(2048);
    vector<u8> oamData(256);
    Image front(256, 240);
    Image back(256, 240);
    u16 v = 0;
    u16 t = 0;
    u8 x = 0;
    bool latch = 0;
    u8 f = 0;
    u8 regis = 0;
    bool nmiOccurred = false;
    bool nmiOutput = true;
    bool nmiPrevious = false;
    u8 nmiDelay = 0;
    u8 nametableByte = 0;
    u8 attributeTableByte = 0;
    u8 lowTileByte = 0;
    u64 tileData = 0;

    u32 spriteCount = 0;
    vector<u32> spritePatterns(8);
    vector<u8> spritePositions(8);
    vector<u8> spritePriorities(8);
    vector<u8> spriteIndices(8);

    u8 flagNametable = 0;
    u8 flagIncrement = 0;
    u8 flagSpriteTable = 0;
    u8 flagBackgroundTable = 0;
    u8 flagSpriteSize = 0;
    u8 flagMasterSlave = 0;

    u8 flagGrayscale = 0;
    u8 flagShowLeftBackground = 0;
    u8 flagShowLeftSprites = 0;
    u8 flagShowBackground = 0;
    u8 flagShowSprites = 0;
    u8 flagRedTint = 0;
    u8 flagGreenTint = 0;
    u8 flagBlueTint = 0;

    u8 flagSpriteZeroHit = 0;
    u8 flagSpriteOverflow = 0;
    u8 oamAddress = 0;
    u8 bufferedData = 0;

    void init()
    {
        cycle = 340;
        scanline = 240;
        frame = 0;
        write_control(0);
        write_mask(0);
        write_oam_address(0);
    }

    void write_register(u16 addr, u8 value)
    {
        assert(addr >= 0x2000);
        u16 diff = addr - 0x2000;
        addr = 0x2000 + (diff % 8);

        switch (addr)
        {
            case 0x2000: write_control(value); break;    // PPUCTRL
            case 0x2001: write_mask(value); break;       // PPUMASK
            case 0x2002: break;                         // PPUSTATUS
            case 0x2003: write_oam_address(value); break; // OAMADDR
            case 0x2004: write_oam_data(value); break;    // OAMDATA
            case 0x2005: write_scroll(value); break      // 
        }
    }

    // PPUCTRL
    void write_control(u8 value)
    {
        flagNametable = (value >> 0) & 3;
        flagIncrement = (value >> 2) & 1;
        flagSpriteTable = (value >> 3) & 1;
        flagBackgroundTable = (value >> 4) & 1;
        flagSpriteSize = (value >> 5) & 1;
        flagMasterSlave = (value >> 6) & 1;
        nmiOutput = (value >> 7) & 1;
        nmi_change();
        // t: ....BA.. ........ = d: ......BA
        t = (t & 0xF3FF) | ((value & 3) << 10);
    }

    // PPUMASK
    void write_mask(u8 value)
    {
        flagGrayscale = (value >> 0) & 1;
        flagShowLeftBackground = (value >> 1) & 1;
        flagShowLeftSprites = (value >> 2) & 1;
        flagShowBackground = (value >> 3) & 1;
        flagShowSprites = (value >> 4) & 1;
        flagRedTint = (value >> 5) & 1;
        flagGreenTint = (value >> 6) & 1;
        flagBlueTint = (value >> 7) & 1;
    }

    void write_oam_address(u8 value)
    {
        oamAddress = value;
    }

    void write_oam_data(u8 value)
    {
        oamData[oamAddress++] = value;
    }

    void write_scroll(u8 value)
    {
        if (!latch)
        {

        }
        else
        {

        }
    }

    u16 nametable_mirror(u16 addr)
    {
        if (addr < 0x2800) return addr;
        else return addr - 0x800;
    }

    void tick()
    {
        if (nmiDelay > 0)
        {
            nmiDelay--;
            if (nmiDelay == 0 && nmiOutput && nmiOccurred)
            {
                // std::cout << "vertical blanking..." << std::endl;
                CPU::trigger_nmi();
            }
        }

        if (flagShowBackground != 0 || flagShowSprites != 0)
        {
            if (f == 1 && scanline == 261 && cycle == 339)
            {
                cycle = 0;
                scanline = 0;
                frame++;
                f ^= 1;
                return;
            }
        }

        cycle++;

        if (cycle > 341)
        {
            cycle = 0;
            scanline++;
            if (scanline > 261)
            {
                scanline = 0;
                frame++;
                f ^= 1;
            }
        }
    }

    // PPUSTATUS
    u8 read_status()
    {
        u8 result = 0;
        result |= regis & 0x1F;
        result |= flagSpriteOverflow << 5;
        result |= flagSpriteZeroHit << 6;

        if (nmiOccurred) result |= 1 << 7;

        nmiOccurred = 0;
        nmi_change();
        latch = false;
        return result; 
    }

    inline u32 color_map(u8 h)
    {
        switch (h)
        {
            case 0: return 0;
            case 1: return 0x333333;
            case 2: return 0x999999;
            case 3: return 0xCCCCCC;
            default: assert(0);
        }
    }

    inline void evaluate_tile(u32 tlx, u32 tly, u32 pattern_idx)
    {
        assert(pattern_idx < 256);

        for (u16 y = 0; y < 8; y++)
        {
            u8 lo = read(pattern_idx * 16 + y);
            u8 hi = read(pattern_idx * 16 + y + 8);
            for (u32 x = 0; x < 8; x++)
            {
                u32 df = 7 - x;
                u8 pix = ((lo >> df) & 1) | (((hi >> df) & 1) << 1);
                back.setPixel(tlx + x, tly + y, color_map(pix));
            }
        }
    }

    void step()
    {
        tick();

        bool rendering_enabled = flagShowBackground || flagShowSprites;
        bool pre_line = scanline == 261;
        bool visible_line = scanline < 240;
        bool render_line = pre_line || visible_line;
        bool pre_fetch_cycle = (321 <= cycle) && (cycle <= 336);
        bool visible_cycle = (cycle >= 1) && (cycle <= 256);
        bool fetch_cycle = pre_fetch_cycle || visible_cycle;

        // std::cout << "(" << scanline << ", " << cycle << ")" << std::endl;

        if (scanline == 241 && cycle == 1) set_vertical_blank();

        if (pre_line && cycle == 1)
        {
            clear_vertical_blank();
            flagSpriteZeroHit = 0;
            flagSpriteOverflow = 0;
        }
    }

    void nmi_change()
    {
        u8 nmi = nmiOutput && nmiOccurred;
        if (nmi && !nmiPrevious) nmiDelay = 1;
        nmiPrevious = nmi;
    }

    void set_vertical_blank()
    {
        swap_frame_buffer();
        nmiOccurred = 1;
        nmi_change();
    }

    void clear_vertical_blank()
    {
        nmiOccurred = false;
        nmi_change();
    }

    u8 read_palette(u16 addr)
    {
        if (addr >= 16 && addr % 4 == 0) addr -= 16;

        return paletteData[addr];
    }

    void write_palette(u16 addr, u8 value)
    {
        if (addr >= 16 && addr % 4 == 0) addr -= 16;

        paletteData[addr] = value;
    }

    void swap_frame_buffer()
    {
        for (u32 y = 0; y < 240; y += 8)
        {
            for (u32 x = 0; x < 256; x += 8)
            {
                u16 base = 0x2000 + (x / 128) * 0x400 + (y / 120) * 0x800;
                u8 pattern_idx = read(base + (x % 128) / 8 + ((y % 120) / 8) * 16);
                evaluate_tile(x, y, pattern_idx);
            }
        }

        back.setPixel(256 - 1, 240 - 1, f ? 0xFFFFFF : 0);

        // std::swap(front, back);
        Display::loadImage(back);
    }
}