#include <memory>
#include <cstdio>
#include "console.hpp"
#include "ppu.hpp"
#include "mapper.hpp"
#include "cartridge.hpp"
#include "cpu.hpp"
#include "display.hpp"
#include "input.hpp"
#include "SDL2/SDL.h"
#include "config.hpp"
#include <chrono>
#include <exception>
#include <thread>

const u64 MAX_FRAME_JUMP = 2;
const u64 DESYNC_MAX_WRAPAROUND = 10; 
    // if we're behind by more frames than this
    // reset the count
const double PRINT_DELAY = 1 / 1.;

namespace Console
{
    array<u8, CONSOLE_RAM_BYTES> ram;
    std::unique_ptr<Mapper> mapper;

    bool init(const string& fileName)
    {
        Cartridge::init(fileName);
        mapper = std::move(Mapper::generateMapper());
        CPU::init();
        PPU::init();
        Display::init();
        return true;
    }

    void run()
    {
        auto program_start = std::chrono::high_resolution_clock::now();
        auto current_time = program_start;
        bool quit = false;
        u64 frame_count_offset = 0;
        auto last_print = program_start;
        while (!quit)
        {
            current_time = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> diff = current_time - program_start;
            std::chrono::duration<double> since_last_print = current_time - last_print;
            double seconds_passed = diff.count();
            u64 frame_render_target = static_cast<u64>(seconds_passed * Config::FRAMERATE) 
                                                        - frame_count_offset;
            if (PPU::frame_count > frame_render_target)
                throw std::runtime_error("game clock ahead of system clock");
            u64 frames_to_go = frame_render_target - PPU::frame_count;
            u64 render_this_tick = std::min<u64>(3, frames_to_go);
            if (frames_to_go > DESYNC_MAX_WRAPAROUND)
            {
                frame_count_offset += frames_to_go - DESYNC_MAX_WRAPAROUND;
                frames_to_go = DESYNC_MAX_WRAPAROUND;
            }
            if (frames_to_go > MAX_FRAME_JUMP)
            {
                if (since_last_print.count() > PRINT_DELAY)
                {
                    printf("Running too slow: %ld frames behind system clock\n", frames_to_go);
                    last_print = current_time;
                }
            }

            u64 new_frame_target = PPU::frame_count + render_this_tick;

            while (PPU::frame_count < new_frame_target)
            {
                step();
            }
            
            if (render_this_tick == 0) SDL_Delay(1);

            SDL_Event event;
            while (SDL_PollEvent(&event))
            {
                switch (event.type)
                {
                    case SDL_QUIT:
                        quit = true;
                        break;
                    case SDL_KEYDOWN:
                    case SDL_KEYUP:
                        Input::handle_event(event);
                    default:
                        break;
                }
            }
        }
    }

    void deinit()
    {
        Display::deinit();
    }

    u32 step()
    {
        // CPU::printInstruction();
        u32 cpu_cycles = CPU::step();
        u32 to_run_ppu = 3 * cpu_cycles;

        for (u32 i = 0; i < to_run_ppu; i++)
        {
            PPU::tick();
        }

        return cpu_cycles;
    }
}