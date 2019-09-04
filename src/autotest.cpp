#include "autotest.hpp"
#include <array>

struct ScheduledInput
{
    const u32 frameNumber;
    const u32 buttonIndex;
    ScheduledInput(u32 frameNumber, u32 buttonIndex) :
        frameNumber(frameNumber), buttonIndex(buttonIndex)
    {

    }
};

Autotest::Autotest(const string &testName, 
                   const string &fileName,
                   const u32 success_display_hash)
        : testName(testName), fileName(fileName), 
          success_display_hash(success_display_hash)
{

}

const std::array<Autotest, 7> TEST_LIST {
    Autotest("NEStest standard opcodes", "roms/nestest.nes", 0x8FED0975),
    Autotest("NEStest illegal opcodes", "roms/nestest.nes", 0x09FE3771),
    Autotest("power up palettes", "roms/power_up_palette.nes", 0x79750CFB),
    Autotest("VRAM access", "roms/vram_access.nes", 0x79750CFB),
    Autotest("sprite RAM", "roms/sprite_ram.nes", 0x79750CFB),
    Autotest("palette RAM", "roms/palette_ram.nes", 0x79750CFB),
    Autotest("VBLANK clear time", "roms/vbl_clear_time", 0x79750CFB)
};