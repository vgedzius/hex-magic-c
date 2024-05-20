#ifndef HEX_PLATFORM_H_
#define HEX_PLATFORM_H_

struct OffScreenBuffer
{
    void *pixels;
    int width, height;
    int bytesPerPixel;
    int pitch;
};

void ClearOffScreenBuffer(OffScreenBuffer *buffer);

#endif