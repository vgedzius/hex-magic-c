#include <stdint.h>
#include "hex_platform.h"

void ClearOffScreenBuffer(OffScreenBuffer *buffer)
{
    int32_t *pixel = (int32_t *)buffer->pixels;
    for (int i = 0; i < buffer->width * buffer->height; ++i)
    {
        *pixel = 0x00000000;
    }
}