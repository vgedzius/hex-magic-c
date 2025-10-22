#include "hex_magic.h"
#if !defined(HEX_MAGIC_RENDER)

#include "hex_magic_platform.h"
#include "hex_magic_math.h"

struct BilinearSample
{
    uint32 a, b, c, d;
};

enum RendererEntryType
{
    RENDERER_ENTRY_CLEAR,
    RENDERER_ENTRY_RECTANGLE,
    RENDERER_ENTRY_HEX,
    RENDERER_ENTRY_BITMAP,
};

struct RendererBasis
{
    V2 position;
};

struct RendererEntryHeader
{
    RendererEntryType type;
};

struct RendererEntryClear
{
    RendererEntryHeader header;
    V4 color;
};

struct RendererEntryRectangle
{
    RendererEntryHeader header;
    RendererBasis basis;

    V2 position;
    V2 dimensions;
    V4 color;
};

struct RendererEntryHex
{
    RendererEntryHeader header;
    RendererBasis basis;

    V2 position;
    V4 color;
    Bitmap *texture;
};

struct RendererEntryBitmap
{
    RendererEntryHeader header;
    RendererBasis basis;

    V2 position;
    Bitmap *bitmap;
};

struct Renderer
{
    real32 scale;

    MemoryIndex maxPushBufferSize;
    MemoryIndex pushBufferSize;

    uint8 *pushBufferBase;
};

#define HEX_MAGIC_RENDER
#endif
