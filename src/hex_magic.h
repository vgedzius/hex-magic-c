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

enum Biome
{
    GRASS = 1,
    DIRT,
    LAVA,
    ROUGH,
    SAND,
    SNOW,
    WATER,
    ROCK
};

struct HexCell
{
    HexCoord coord;
    V2 position;

    Biome biome;
};

struct World
{
    V2 position;
    real32 scale;

    int32 width;
    int32 height;

    HexCell *cells;
    HexCell *selectedCell;
};

struct Camera
{
    V2 position;
    V2 velocity;

    real32 speed;
};

enum GameMode
{
    EDIT,
    PLAY
};

struct Editor
{
    Biome selectedBiome;
};

struct GameState
{
    MemoryArena worldArena;
    World *world;

    Camera camera;

    GameMode mode;
    Editor editor;
};

#define HEX_MAGIC_H
#endif
