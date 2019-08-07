#include "display.hpp"
#include <algorithm>

const u32 DISPLAY_WIDTH = 256;
const u32 DISPLAY_HEIGHT = 240;

constexpr u32 rgb_to_u32(u8 r, u8 g, u8 b)
{
    return (r << 16) | (g << 8) | (b << 0);
}

const u8 get_r(u32 rgb)
{
    return (rgb >> 16) & 0xFF;
}

const u8 get_g(u32 rgb)
{
    return (rgb >> 8) & 0xFF;
}

const u8 get_b(u32 rgb)
{
    return (rgb >> 0) & 0xFF;
}



constexpr u32 index(u32 u, u32 v)
{
    return u + v * DISPLAY_WIDTH;
}

namespace Display
{
    array<u32, DISPLAY_WIDTH * DISPLAY_HEIGHT> buffer;

    void writePixel(u32 u, u32 v, u32 rgb)
    {
        buffer[index(u, v)] = rgb;
    }

    void writePixel(u32 u, u32 v, u8 r, u8 g, u8 b)
    {
        writePixel(u, v, rgb_to_u32(r, g, b));
    }

    void fill(u32 rgb)
    {
        std::fill(buffer.begin(), buffer.end(), rgb);
    }

    void fill(u8 r, u8 g, u8 b)
    {
        fill(rgb_to_u32(r, g, b));
    }

    void clear()
    {
        fill(0, 0, 0);
    }

    /* Flip the buffer to the display */
    void render()
    {
        // u32 *ptr = buffer.data();
    }

    bool init()
    {
        return true;
    }

    void deinit()
    {

    }

    void buffer_to_file(const string &file_name)
    {
        FILE *fp = fopen(file_name.c_str(), "wb");
        fprintf(fp, "P6\n%d %d\n255\n", DISPLAY_WIDTH, DISPLAY_HEIGHT);
        for (u32 j = 0; j < DISPLAY_HEIGHT; j++)
        {
            for (u32 i = 0; i < DISPLAY_WIDTH; i++)
            {
                static u8 color[3];
                u32 rgb = buffer[index(i, j)];
                color[0] = get_r(rgb);
                color[1] = get_g(rgb);
                color[2] = get_b(rgb);
                fwrite(color, 1, 3, fp);
            }
        }
        fclose(fp);
    }
}