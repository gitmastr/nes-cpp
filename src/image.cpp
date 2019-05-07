#include <algorithm>
#include "image.hpp"

Image::Image(u32 width, u32 height) :
    width(width), height(height), buffer(width * height)
{
    fill(0);
}

Image::~Image()
{
    
}

void Image::setPixel(u32 u, u32 v, u32 rgb)
{
    buffer[index(u, v)] = rgb;
}

void Image::setPixel(u32 u, u32 v, u8 r, u8 g, u8 b)
{
    buffer[index(u, v)] = rgb(r, g, b);
}

void Image::fill(u8 r, u8 g, u8 b)
{
    fill(rgb(r, g, b));
}

void Image::fill(u32 rgb)
{
    std::fill(buffer.begin(), buffer.end(), rgb);
}

u32 Image::getPixel(u32 u, u32 v) { return buffer[index(u, v)]; }
u32 *Image::getBuffer() { return buffer.data(); }

u32 Image::index(u32 u, u32 v)
{
    return u + v * width;
}

u32 Image::rgb(u8 r, u8 g, u8 b)
{
    return (r << 16) | (g << 8) | (b << 0);
}