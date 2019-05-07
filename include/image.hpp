#pragma once

#include "types.hpp"

// An RGB image
class Image
{
public:
    Image(u32 width, u32 height);
    ~Image();
    void setPixel(u32 u, u32 v, u32 rgb);
    void setPixel(u32 u, u32 v, u8 r, u8 g, u8 b);
    void fill(u8 r, u8 g, u8 b);
    void fill(u32 rgb);
    u32 getPixel(u32 u, u32 v);
    u32 *getBuffer();
    const u32 width;
    const u32 height;
private:
    vector<u32> buffer;
    inline u32 index(u32 u, u32 v);
    static inline u32 rgb(u8 r, u8 g, u8 b);
};