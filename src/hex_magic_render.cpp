#include "hex_magic.h"
#include "hex_magic_intrinsics.h"
#include "hex_magic_math.h"
#include "hex_magic_platform.h"
#include "hex_magic_render.h"

inline V2 ScreenToWorld(GameOffscreenBuffer *buffer, Renderer *renderer, int32 x, int32 y)
{
    Camera *camera  = renderer->camera;
    V2 screenCenter = 0.5f * Vector2(buffer->width, buffer->height);

    V2 result = Vector2(x, buffer->height - y);

    result -= screenCenter;
    result *= 1.0f / camera->zoom;
    result += camera->position;

    return result;
}

inline V2 WorldToScreen(GameOffscreenBuffer *buffer, Renderer *renderer, V2 point)
{
    Camera *camera  = renderer->camera;
    V2 screenCenter = 0.5f * Vector2(buffer->width, buffer->height);

    V2 result = point - camera->position;

    // TODO should use proper perspecive calculations here for proper zoom and scaling
    // (1.0f / distanceToCamera) * focalLenght * worldPosition

    result *= camera->zoom;
    result += screenCenter;
    result.y = buffer->height - result.y;

    return result;
}

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

internal Renderer *MakeRenderer(MemoryArena *arena, MemoryIndex maxPushBufferSize, Camera *camera)
{
    Renderer *renderer = PushStruct(arena, Renderer);

    renderer->maxPushBufferSize = maxPushBufferSize;
    renderer->pushBufferSize    = 0;
    renderer->pushBufferBase    = (uint8 *)PushSize(arena, maxPushBufferSize);
    renderer->camera            = camera;

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

internal void RendererPushRectangle(Renderer *renderer, V2 position, V2 dimensions, V4 color)
{
    RendererEntryRectangle *entry = PushRenderElement(renderer, RendererEntryRectangle, RENDERER_ENTRY_RECTANGLE);

    if (entry)
    {
        entry->position   = position;
        entry->dimensions = dimensions;
        entry->color      = color;
    }
}

internal void RendererPushHex(Renderer *renderer, V2 position, V4 color, Bitmap *texture)
{
    RendererEntryHex *entry = PushRenderElement(renderer, RendererEntryHex, RENDERER_ENTRY_HEX);

    if (entry)
    {
        entry->position = position;
        entry->color    = color;
        entry->texture  = texture;
    }
}

internal void RendererPushBitmap(Renderer *renderer, V2 position, Bitmap *bitmap)
{
    RendererEntryBitmap *entry = PushRenderElement(renderer, RendererEntryBitmap, RENDERER_ENTRY_BITMAP);

    if (entry)
    {
        entry->position = position;
        entry->bitmap   = bitmap;
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

    uint8 *row = (uint8 *)buffer->memory + minX * BITMAP_BYTES_PER_PIXEL + minY * buffer->pitch;
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

inline V4 Unpack(uint32 value)
{
    V4 result = {
        (real32)((value >> 16) & 0xFF),
        (real32)((value >> 8) & 0xFF),
        (real32)((value >> 0) & 0xFF),
        (real32)((value >> 24) & 0xFF),
    };

    return result;
}

inline uint32 Pack(V4 value)
{
    uint32 result = (((uint32)(value.a + 0.5f) << 24) | ((uint32)(value.r + 0.5f) << 16) |
                     ((uint32)(value.g + 0.5f) << 8) | ((uint32)(value.b + 0.5f) << 0));

    return result;
}

inline BilinearSample SampleBilinear(Bitmap *texture, int32 x, int32 y)
{
    BilinearSample result;

    uint8 *texelPtr = (uint8 *)texture->memory + y * texture->pitch + x * sizeof(uint32);

    result.a = *(uint32 *)(texelPtr);
    result.b = *(uint32 *)(texelPtr + sizeof(uint32));
    result.c = *(uint32 *)(texelPtr + texture->pitch);
    result.d = *(uint32 *)(texelPtr + texture->pitch + sizeof(uint32));

    return result;
}

inline V4 BilinearBlend(BilinearSample sample, real32 fX, real32 fY)
{
    V4 texelA = Unpack(sample.a);
    V4 texelB = Unpack(sample.b);
    V4 texelC = Unpack(sample.c);
    V4 texelD = Unpack(sample.d);

    V4 result = Lerp(Lerp(texelA, texelB, fX), Lerp(texelC, texelD, fX), fY);

    return result;
}

internal void DrawHex(GameOffscreenBuffer *buffer, Renderer *renderer, V2 worldPosition, V4 color, Bitmap *texture)
{
    real32 sqrt3      = Sqrt(3);
    Camera *camera    = renderer->camera;
    real32 scale      = camera->zoom;
    V2 screenCenter   = {0.5f * buffer->width, 0.5f * buffer->height};
    V2 screenPosition = WorldToScreen(buffer, renderer, worldPosition);

    int32 minX = RoundReal32ToInt32(screenPosition.x - sqrt3 / 2.0f * scale);
    int32 maxX = RoundReal32ToInt32(screenPosition.x + sqrt3 / 2.0f * scale);
    int32 minY = RoundReal32ToInt32(screenPosition.y - scale);
    int32 maxY = RoundReal32ToInt32(screenPosition.y + scale);

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

    real32 v = 0.5 * scale;
    real32 h = 0.5 * sqrt3 * scale;

    real32 invTextureWorldSize = 0.5;

    uint8 *destRow = (uint8 *)buffer->memory + minX * BITMAP_BYTES_PER_PIXEL + minY * buffer->pitch;
    for (int32 y = minY; y < maxY; ++y)
    {
        uint32 *dest = (uint32 *)destRow;

        for (int32 x = minX; x < maxX; ++x)
        {
            real32 q2x = Abs(x - screenPosition.x);
            real32 q2y = Abs(y - screenPosition.y);

            if (2 * v * h - v * q2x - h * q2y >= 0)
            {
                V2 pixelWorldP = ScreenToWorld(buffer, renderer, x, y);
                V2 uv          = invTextureWorldSize * Vector2(-pixelWorldP.y, pixelWorldP.x);

                uv.x -= FloorReal32ToInt32(uv.x);
                uv.y -= FloorReal32ToInt32(uv.y);

                real32 tX = uv.x * (texture->width - 1);
                real32 tY = uv.y * (texture->height - 1);

                Assert(tX >= 0 && tX < texture->width);
                Assert(tY >= 0 && tY < texture->height);

                int32 tXi = (int32)tX;
                int32 tYi = (int32)tY;

                real32 fX = tX - (real32)tXi;
                real32 fY = tY - (real32)tYi;

                BilinearSample source = SampleBilinear(texture, tYi, tXi);
                V4 texel              = BilinearBlend(source, fX, fY);

                if (color.a > 0.0)
                {
                    // TODO temp hack, should decide how we want to implement the tint
                    texel = Lerp(texel, color, color.a);
                }

                V4 d      = Unpack(*dest);
                V4 result = (1.0f - texel.a / 255.0f) * d + texel;
                *dest     = Pack(result);
            }

            ++dest;
        }

        destRow += buffer->pitch;
    }
}

internal void DrawBitmap(GameOffscreenBuffer *buffer, Bitmap *bitmap, V2 origin, V2 xAxis, V2 yAxis, V4 color)
{
    color.rgb *= color.a;

    real32 invXAxisLengthSq = 1.0f / LengthSq(xAxis);
    real32 invYAxisLengthSq = 1.0f / LengthSq(yAxis);

    int32 widthMax  = buffer->width - 1;
    int32 heightMax = buffer->height - 1;

    real32 invWidthMax  = 1.0f / (real32)widthMax;
    real32 invHeightMax = 1.0f / (real32)heightMax;

    int32 xMin = widthMax;
    int32 xMax = 0;
    int32 yMin = heightMax;
    int32 yMax = 0;

    V2 p[4] = {
        origin,
        origin + xAxis,
        origin + xAxis + yAxis,
        origin + yAxis,
    };

    for (uint32 pIndex = 0; pIndex < ArrayCount(p); ++pIndex)
    {
        V2 testP     = p[pIndex];
        int32 floorX = FloorReal32ToInt32(testP.x);
        int32 ceilX  = CeilReal32ToInt32(testP.x);
        int32 floorY = FloorReal32ToInt32(testP.y);
        int32 ceilY  = CeilReal32ToInt32(testP.y);

        if (xMin > floorX)
        {
            xMin = floorX;
        }

        if (yMin > floorY)
        {
            yMin = floorY;
        }

        if (xMax < ceilX)
        {
            xMax = ceilX;
        }

        if (yMax < ceilY)
        {
            yMax = ceilY;
        }
    }

    if (xMin < 0)
    {
        xMin = 0;
    }

    if (yMin < 0)
    {
        yMin = 0;
    }

    if (xMax > widthMax)
    {
        xMax = widthMax;
    }

    if (yMax > heightMax)
    {
        yMax = heightMax;
    }

    uint8 *row = (uint8 *)buffer->memory + xMin * BITMAP_BYTES_PER_PIXEL + yMin * buffer->pitch;
    for (int32 y = yMin; y <= yMax; ++y)
    {
        uint32 *pixel = (uint32 *)row;
        for (int32 x = xMin; x <= xMax; ++x)
        {
            V2 pixelP = Vector2(x, y);
            V2 d      = pixelP - origin;

            real32 edge0 = Inner(d, -Perp(xAxis));
            real32 edge1 = Inner(d - xAxis, -Perp(yAxis));
            real32 edge2 = Inner(d - xAxis - yAxis, Perp(xAxis));
            real32 edge3 = Inner(d - yAxis, Perp(yAxis));

            if (edge0 < 0.0f && edge1 < 0.0f && edge2 < 0.0f && edge3 < 0.0f)
            {
                V2 screenSpaceUV = {invWidthMax * x, invHeightMax * y};

                real32 u = invXAxisLengthSq * Inner(d, xAxis);
                real32 v = invYAxisLengthSq * Inner(d, yAxis);

                real32 tX = u * (bitmap->width - 2);
                real32 tY = v * (bitmap->height - 2);

                int32 x = (int32)tX;
                int32 y = (int32)tY;

                real32 fX = tX - (real32)x;
                real32 fY = tY - (real32)y;

                Assert(x >= 0 && x < bitmap->width);
                Assert(y >= 0 && y < bitmap->height);

                BilinearSample texelSample = SampleBilinear(bitmap, x, y);
                V4 texel                   = BilinearBlend(texelSample, fX, fY);

                texel      = Hadamard(texel, color);
                V4 dest    = Unpack(*pixel);
                V4 blended = (1.0f - texel.a / 255.0f) * dest + texel;
                *pixel     = Pack(blended);
            }

            ++pixel;
        }

        row += buffer->pitch;
    }
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
                Camera *camera                 = renderer->camera;
                V2 screenPosition              = WorldToScreen(output, renderer, render->position);

                V2 min = {screenPosition.x - render->dimensions.x * 0.5f * camera->zoom,
                          screenPosition.y - render->dimensions.y * 0.5f * camera->zoom};

                V2 max = {screenPosition.x + render->dimensions.x * 0.5f * camera->zoom,
                          screenPosition.y + render->dimensions.y * 0.5f * camera->zoom};

                DrawRectangle(output, min, max, render->color);

                baseAddress += sizeof(*render);
            }
            break;

            case RENDERER_ENTRY_HEX:
            {
                RendererEntryHex *render = (RendererEntryHex *)baseEntry;

                DrawHex(output, renderer, render->position, render->color, render->texture);

                baseAddress += sizeof(*render);
            }
            break;

            case RENDERER_ENTRY_BITMAP:
            {
                RendererEntryBitmap *render = (RendererEntryBitmap *)baseEntry;
                Bitmap *bitmap              = render->bitmap;
                Camera *camera              = renderer->camera;
                V2 origin                   = WorldToScreen(output, renderer, render->position);
                real32 scale                = camera->zoom / 150.0;

                origin -= 0.5 * scale * Vector2(bitmap->width, bitmap->height);

                V2 xAxis = scale * Vector2(bitmap->width, 0);
                V2 yAxis = scale * Vector2(0, bitmap->height);

                DrawBitmap(output, render->bitmap, origin, xAxis, yAxis, {1.0, 1.0, 1.0, 1.0});

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
