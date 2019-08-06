#pragma once

#include "types.hpp"

class SDL_Window;
class SDL_Renderer;
class SDL_Texture;

class Image;

namespace Display
{
    extern const u32 WIDTH;
    extern const u32 HEIGHT;
    extern const string TITLE;

    extern SDL_Window *window;
    extern SDL_Renderer *renderer;
    extern SDL_Texture *texture;

    void init(u32 scalar);
    void deinit();
    void loadImage(Image &image);

}