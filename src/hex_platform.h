#ifndef HEX_PLATFORM_H_
#define HEX_PLATFORM_H_

#include <SDL2/SDL.h>

struct Texture
{
    SDL_Texture *texture;
    int width, height;
};

struct OffScreenBuffer
{
    void *pixels;
    int width, height;
    int bytesPerPixel;
    int pitch;
};

void ClearOffScreenBuffer(OffScreenBuffer *buffer);

#endif