#if !defined(HEX_MAGIC_H)

#include "hex_magic_platform.h"
#include "hex_magic_math.h"
#include "hex_magic_intrinsics.h"
#include "hex_magic_hex.h"

struct MemoryArena
{
    MemoryIndex size;
    uint8 *base;
    MemoryIndex used;
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

struct Cell
{
    HexCoord coord;
    V2 position;

    Biome biome;
    uint32 cityIndex;
    uint32 heroIndex;
};

enum EntityType
{
    ENTITY_HERO,
    ENTITY_CITY,
};

struct Entity
{
    V2 position;
    EntityType type;
};

struct World
{
    int32 width;
    int32 height;

    Cell *cells;
    Cell *selectedCell;

    uint32 entityCount;
    Entity entities[256];
};

struct Camera
{
    V2 position;
    V2 velocity;
    real32 speed;
    real32 friction;

    real32 zoomVelocity;
    real32 zoom;
    real32 zoomSpeed;
    real32 zoomFriction;
    real32 minZoom;
    real32 maxZoom;
};

enum GameMode
{
    PLAY,
    EDIT
};

enum BrushType
{
    BRUSH_BIOME,
    BRUSH_ENTITY,
};

struct Editor
{
    BrushType brush;
    Biome brushBiome;

    uint32 brushSize;
    uint32 minBrushSize;
    uint32 maxBrushSize;

    EntityType brushEntity;
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
