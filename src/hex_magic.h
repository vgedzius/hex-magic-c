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

struct HexCoord
{
    int32 q, r, s;
};

struct HexCoordF
{
    real32 q, r, s;
};

struct OffsetCoord
{
    int32 x, y;
};

struct Color
{
    real32 r, g, b;
};

struct Cell
{
    HexCoord coord;
    V2 position;

    Color color;
};

struct World
{
    V2 position;
    real32 scale;

    int32 width;
    int32 height;

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
