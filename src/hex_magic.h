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
    WATER,
    GRASS,
    DIRT,
    LAVA,
    ROUGH,
    SAND,
    SNOW,
    ROCK
};

struct HexCell
{
    HexCoord coord;
    V2 position;

    Biome biome;
    uint32 heroIndex;
};

struct Hero
{
    bool32 alive;
};

struct World
{
    V2 position;
    real32 scale;

    int32 width;
    int32 height;

    HexCell *cells;
    HexCell *selectedCell;

    uint32 heroCount;
    Hero heroes[256];
};

struct Camera
{
    V2 position;
    V2 velocity;

    real32 speed;
};

enum GameMode
{
    PLAY,
    EDIT
};

enum BrushType
{
    BIOME,
    HERO
};

struct Editor
{
    BrushType brush;
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
