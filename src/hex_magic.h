#if !defined(HEX_MAGIC)

#include "hex_magic_platform.h"
#include "hex_magic_math.h"
#include "hex_magic_intrinsics.h"
#include "hex_magic_hex.h"

#define BITMAP_BYTES_PER_PIXEL 4

struct MemoryArena
{
    MemoryIndex size;
    uint8 *base;
    MemoryIndex used;

    uint32 tempCount;
};

struct TemporaryMemory
{
    MemoryArena *arena;
    MemoryIndex used;
};

#define PushStruct(arena, type) (type *)PushSize_(arena, sizeof(type))
#define PushArray(arena, count, type) (type *)PushSize_(arena, count * sizeof(type))
#define PushSize(arena, size) PushSize_(arena, size)

inline void *PushSize_(MemoryArena *arena, MemoryIndex size)
{
    Assert((arena->used + size) <= arena->size);

    void *result = arena->base + arena->used;
    arena->used += size;

    return result;
}

enum Biome
{
    WATER,
    GRASS,
    DIRT,
    LAVA,
    ROUGH,
    SAND,
    SNOW,
    SWAMP,
    ROCK
};

struct Cell
{
    HexCoord coord;
    V2 position;

    Biome biome;

    uint32 cityIndex;
    uint32 heroIndex;
    uint32 resourceIndex;
};

enum EntityType
{
    ENTITY_HERO,
    ENTITY_RESOURCE,
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

struct Bitmap
{
    int32 width;
    int32 height;
    int32 pitch;
    void *memory;
};

struct GameState
{
    MemoryArena worldArena;
    World *world;

    Camera camera;

    GameMode mode;
    Editor editor;

    Bitmap city;
    Bitmap hero;

    Bitmap grassTexture;
    Bitmap dirtTexture;
    Bitmap lavaTexture;
    Bitmap roughTexture;
    Bitmap sandTexture;
    Bitmap snowTexture;
    Bitmap swampTexture;
    Bitmap waterTexture;
    Bitmap rockTexture;
};

struct TransientState
{
    bool32 isInitialized;
    MemoryArena transientArena;
};

#define HEX_MAGIC
#endif
