#if !defined(HEX_MAGIC_H)

#include "hex_magic_platform.h"
#include "hex_magic_math.h"
#include "hex_magic_intrinsics.h"

struct MemoryArena
{
    MemoryIndex size;
    uint8 *base;
    MemoryIndex used;
};

struct HexMetrics
{
    real32 outerRadius;
    real32 innerRadius;

    V2 corners[6];
};

struct HexCoord
{
    int32 x, y, z;
};

struct Color
{
    real32 r, g, b;
};

struct Cell
{
    HexCoord coord;
    Color color;
    V2 position;
};

struct World
{
    HexMetrics metrics;
    V2 position;

    uint32 width;
    uint32 height;

    Cell *cells;
};

struct GameState
{
    MemoryArena worldArena;
    World *world;

    V2 cameraPos;
};

#define HEX_MAGIC_H
#endif
