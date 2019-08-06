#include "display.hpp"
#include "image.hpp"
#include "SDL2/SDL.h"
#include <cassert>

namespace Display
{
    const string TITLE = "nes.xlsx";
    const u32 WIDTH = 256;
    const u32 HEIGHT = 240;

    SDL_Window *window = nullptr;
    SDL_Renderer *renderer = nullptr;
    SDL_Texture *texture = nullptr;

    void init(u32 scalar)
    {
        if (SDL_Init(SDL_INIT_VIDEO) < 0) throw "SDL init error";

        window = SDL_CreateWindow(TITLE.c_str(),
                                  SDL_WINDOWPOS_UNDEFINED,
                                  SDL_WINDOWPOS_UNDEFINED,
                                  scalar * WIDTH, scalar * HEIGHT, 0);

        if (window == NULL) throw "SDL window error";

        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC); // | SDL_RENDERER_PRESENTVSYNC);

        if (renderer == NULL) throw "SDL renderer error";

        texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888,
                                    SDL_TEXTUREACCESS_STREAMING,
                                    WIDTH, HEIGHT);
    }

    void deinit()
    {
        SDL_RenderClear(renderer);
        SDL_DestroyTexture(texture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }

    void loadImage(Image &image)
    {
        assert(image.width == WIDTH && image.height == HEIGHT);

        assert(SDL_UpdateTexture(texture, nullptr, image.getBuffer(), WIDTH * sizeof(u32)) == 0);
        assert(SDL_RenderClear(renderer) == 0);
        assert(SDL_RenderCopy(renderer, texture, nullptr, nullptr) == 0);
        SDL_RenderPresent(renderer);
    }
}