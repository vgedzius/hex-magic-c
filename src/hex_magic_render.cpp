#include "hex_magic.h"
#include "hex_magic_platform.h"
#include "hex_magic_render.h"

#define PushRenderElement(renderer, element, type) (element *)PushRenderElement_(renderer, sizeof(element), type)
inline RendererEntryHeader *PushRenderElement_(Renderer *renderer, MemoryIndex size, RendererEntryType type)
{
    RendererEntryHeader *result = 0;

    if (renderer->pushBufferSize + size < renderer->maxPushBufferSize)
    {
        result       = (RendererEntryHeader *)(renderer->pushBufferBase + renderer->pushBufferSize);
        result->type = type;

        renderer->pushBufferSize += size;
    }

    return result;
}

internal Renderer *MakeRenderer(MemoryArena *arena, MemoryIndex maxPushBufferSize, real32 scale)
{
    Renderer *renderer = PushStruct(arena, Renderer);

    renderer->maxPushBufferSize = maxPushBufferSize;
    renderer->pushBufferSize    = 0;
    renderer->pushBufferBase    = (uint8 *)PushSize(arena, maxPushBufferSize);
    renderer->scale             = scale;

    return renderer;
}

internal void RendererClear(Renderer *renderer, V4 color)
{
    RendererEntryClear *entry = PushRenderElement(renderer, RendererEntryClear, RENDERER_ENTRY_CLEAR);

    if (entry)
    {
        entry->color = color;
    }
}

internal void RendererDrawRectangle(Renderer *renderer, V2 basePosition, V2 position, V2 dimensions, V4 color)
{
    RendererEntryRectangle *entry = PushRenderElement(renderer, RendererEntryRectangle, RENDERER_ENTRY_RECTANGLE);

    if (entry)
    {
        entry->basis.position = basePosition;
        entry->position       = position;
        entry->dimensions     = dimensions;
        entry->color          = color;
    }
}

internal void RendererDrawHex(Renderer *renderer, V2 basePosition, V2 position, V4 color)
{
    RendererEntryHex *entry = PushRenderElement(renderer, RendererEntryHex, RENDERER_ENTRY_HEX);

    if (entry)
    {
        entry->basis.position = basePosition;
        entry->position       = position;
        entry->color          = color;
    }
}

internal void RendererDrawBitmap(Renderer *renderer, V2 basePosition, V2 position, LoadedBitmap *bitmap)
{
    RendererEntryBitmap *entry = PushRenderElement(renderer, RendererEntryBitmap, RENDERER_ENTRY_BITMAP);

    if (entry)
    {
        entry->basis.position = basePosition;
        entry->position       = position;
        entry->bitmap         = bitmap;
    }
}

internal void DrawRectangle(GameOffscreenBuffer *buffer, V2 vMin, V2 vMax, V4 c)
{
    int32 minX = RoundReal32ToInt32(vMin.x);
    int32 minY = RoundReal32ToInt32(vMin.y);
    int32 maxX = RoundReal32ToInt32(vMax.x);
    int32 maxY = RoundReal32ToInt32(vMax.y);

    if (minX < 0)
    {
        minX = 0;
    }

    if (minY < 0)
    {
        minY = 0;
    }

    if (maxX > buffer->width)
    {
        maxX = buffer->width;
    }

    if (maxY > buffer->height)
    {
        maxY = buffer->height;
    }

    uint32 color = (RoundReal32ToUint32(c.a * 255.0f) << 24) | (RoundReal32ToUint32(c.r * 255.0f) << 16) |
                   (RoundReal32ToUint32(c.g * 255.0f) << 8) | (RoundReal32ToUint32(c.b * 255.0f) << 0);

    uint8 *row = (uint8 *)buffer->memory + minX * buffer->bytesPerPixel + minY * buffer->pitch;
    for (int y = minY; y < maxY; ++y)
    {
        uint32 *pixel = (uint32 *)row;
        for (int x = minX; x < maxX; ++x)
        {
            *pixel++ = color;
        }

        row += buffer->pitch;
    }
}

internal void DrawHex(GameOffscreenBuffer *buffer, V2 pos, real32 size, V4 color)
{
    real32 sqrt3 = Sqrt(3);

    int32 minX = RoundReal32ToInt32(pos.x - sqrt3 / 2.0f * size);
    int32 maxX = RoundReal32ToInt32(pos.x + sqrt3 / 2.0f * size);
    int32 minY = RoundReal32ToInt32(pos.y - size);
    int32 maxY = RoundReal32ToInt32(pos.y + size);

    if (minX < 0)
    {
        minX = 0;
    }

    if (maxX > buffer->width)
    {
        maxX = buffer->width;
    }

    if (minY < 0)
    {
        minY = 0;
    }

    if (maxY > buffer->height)
    {
        maxY = buffer->height;
    }

    real32 v = 0.5f * size;
    real32 h = sqrt3 / 2 * size;

    uint8 *destRow = (uint8 *)buffer->memory + minX * buffer->bytesPerPixel + minY * buffer->pitch;
    for (int32 y = minY; y < maxY; ++y)
    {
        uint32 *dest = (uint32 *)destRow;

        for (int32 x = minX; x < maxX; ++x)
        {
            real32 q2x = Abs(x - pos.x);
            real32 q2y = Abs(y - pos.y);

            if (2 * v * h - v * q2x - h * q2y >= 0)
            {
                uint32 cellColor =
                    (RoundReal32ToUint32(color.a * 255.0f) << 24) | (RoundReal32ToUint32(color.r * 255.0f) << 16) |
                    (RoundReal32ToUint32(color.g * 255.0f) << 8) | (RoundReal32ToUint32(color.b * 255.0f) << 0);

                *dest = cellColor;
            }

            ++dest;
        }

        destRow += buffer->pitch;
    }
}

internal void DrawBitmap(GameOffscreenBuffer *buffer, LoadedBitmap *bitmap, V2 position)
{
    int32 minX = RoundReal32ToInt32(position.x);
    int32 minY = RoundReal32ToInt32(position.y);
    int32 maxX = minX + bitmap->width;
    int32 maxY = minY + bitmap->height;

    int32 sourceOffsetX = 0;
    int32 sourceOffsetY = 0;

    if (minX < 0)
    {
        sourceOffsetX = -minX;
        minX          = 0;
    }

    if (minY < 0)
    {
        sourceOffsetY = -minY;
        minY          = 0;
    }

    if (maxX > buffer->width)
    {
        maxX = buffer->width;
    }

    if (maxY > buffer->height)
    {
        maxY = buffer->height;
    }

    uint8 *sourceRow = (uint8 *)bitmap->memory + sourceOffsetY * bitmap->pitch + BITMAP_BYTES_PER_PIXEL * sourceOffsetX;
    uint8 *destRow   = (uint8 *)buffer->memory + minX * BITMAP_BYTES_PER_PIXEL + minY * buffer->pitch;

    for (int32 y = minY; y < maxY; ++y)
    {
        uint32 *dest   = (uint32 *)destRow;
        uint32 *source = (uint32 *)sourceRow;

        for (int32 x = minX; x < maxX; ++x)
        {
            V4 texel = {
                (real32)((*source >> 16) & 0xFF),
                (real32)((*source >> 8) & 0xFF),
                (real32)((*source >> 0) & 0xFF),
                (real32)((*source >> 24) & 0xFF),
            };

            V4 d = {
                (real32)((*dest >> 16) & 0xFF),
                (real32)((*dest >> 8) & 0xFF),
                (real32)((*dest >> 0) & 0xFF),
                (real32)((*dest >> 24) & 0xFF),
            };

            V4 result = (1.0f - texel.a / 255.0f) * d + texel;

            *dest = (((uint32)(result.a + 0.5f) << 24) | ((uint32)(result.r + 0.5f) << 16) |
                     ((uint32)(result.g + 0.5f) << 8) | ((uint32)(result.b + 0.5f) << 0));

            ++dest;
            ++source;
        }

        destRow += buffer->pitch;
        sourceRow += bitmap->pitch;
    }
}

internal V2 PointToScreen(GameOffscreenBuffer *buffer, Renderer *renderer, V2 basePosition, V2 point)
{
    V2 screenCenter = {0.5f * (real32)buffer->width, 0.5f * (real32)buffer->height};
    V2 result       = point - basePosition;

    result *= renderer->scale;
    result += screenCenter;
    result.y = buffer->height - result.y;

    return result;
}

internal void RenderToOutput(GameOffscreenBuffer *output, Renderer *renderer)
{
    for (uint32 baseAddress = 0; baseAddress < renderer->pushBufferSize;)
    {
        RendererEntryHeader *baseEntry = (RendererEntryHeader *)(renderer->pushBufferBase + baseAddress);

        switch (baseEntry->type)
        {
            case RENDERER_ENTRY_CLEAR:
            {
                RendererEntryClear *render = (RendererEntryClear *)baseEntry;
                DrawRectangle(output, {0.0f, 0.0f}, {(real32)output->width, (real32)output->height}, render->color);

                baseAddress += sizeof(*render);
            }
            break;

            case RENDERER_ENTRY_RECTANGLE:
            {
                RendererEntryRectangle *render = (RendererEntryRectangle *)baseEntry;
                V2 screenPosition = PointToScreen(output, renderer, render->basis.position, render->position);

                V2 min = {screenPosition.x - render->dimensions.x * 0.5f * renderer->scale,
                          screenPosition.y - render->dimensions.y * 0.5f * renderer->scale};

                V2 max = {screenPosition.x + render->dimensions.x * 0.5f * renderer->scale,
                          screenPosition.y + render->dimensions.y * 0.5f * renderer->scale};

                DrawRectangle(output, min, max, render->color);

                baseAddress += sizeof(*render);
            }
            break;

            case RENDERER_ENTRY_HEX:
            {
                RendererEntryHex *render = (RendererEntryHex *)baseEntry;
                V2 screenPosition        = PointToScreen(output, renderer, render->basis.position, render->position);

                DrawHex(output, screenPosition, renderer->scale, render->color);

                baseAddress += sizeof(*render);
            }
            break;

            case RENDERER_ENTRY_BITMAP:
            {
                RendererEntryBitmap *render = (RendererEntryBitmap *)baseEntry;
                V2 screenPosition           = PointToScreen(output, renderer, render->basis.position, render->position);
                screenPosition -= 0.5f * V2{(real32)render->bitmap->width, (real32)render->bitmap->height};

                DrawBitmap(output, render->bitmap, screenPosition);

                baseAddress += sizeof(*render);
            }
            break;

            default:
            {
                InvalidCodePath;
            }
        }
    }
}
