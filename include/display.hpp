#pragma once

#include "types.hpp"

// contains SDL logic
// and a buffer for frames
namespace Display
{
    void writePixel(u32 u, u32 v, u32 rgb);
    void writePixel(u32 u, u32 v, u8 r, u8 g, u8 b);
    void fill(u32 rgb);
    void fill(u8 r, u8 g, u8 b);
    void clear();
    void render();
    void buffer_to_file(const string &file_name);
    bool init();
    void deinit();
}