#include "hex_magic.h"
#include "hex_magic_intrinsics.h"
#include "hex_magic_platform.h"

internal void InitializeArena(MemoryArena *arena, MemoryIndex size, uint8 *base)
{
    arena->size = size;
    arena->base = base;
    arena->used = 0;
}

#define PushStruct(arena, type) (type *)PushSize_(arena, sizeof(type))
#define PushArray(arena, count, type) (type *)PushSize_(arena, count * sizeof(type))
void *PushSize_(MemoryArena *arena, MemoryIndex size)
{
    Assert((arena->used + size) <= arena->size);

    void *result = arena->base + arena->used;
    arena->used += size;

    return result;
}

internal void GameOutputSound(GameState *gameState, GameSoundOutputBuffer *soundBuffer, int toneHz)
{
    int16 toneVolume = 3000;
    int32 wavePeriod = soundBuffer->samplesPerSecond / toneHz;

    int16 *sampleOut = soundBuffer->samples;
    for (int32 sampleIndex = 0; sampleIndex < soundBuffer->sampleCount; ++sampleIndex)
    {
#if 0
        real32 sineValue  = sinf(gameState->tSine);
        int16 sampleValue = (int16)(sineValue * toneVolume);
#else
        int16 sampleValue = 0;
#endif
        *sampleOut++ = sampleValue;
        *sampleOut++ = sampleValue;

#if 0
        gameState->tSine += 2.0f * PI32 / (real32)wavePeriod;
        if (gameState->tSine > 2.0f * PI32)
        {
            gameState->tSine -= 2.0f * PI32;
        }
#endif
    }
}

inline HexCoord HexCoordFromOffsetCoord(uint32 x, uint32 y)
{
    HexCoord result;

    result.x = x;
    result.y = y;
    result.z = -x - y;

    return result;
}

internal void DrawCell(GameOffscreenBuffer *buffer, GameState *state, Cell *cell)
{
    World *world     = state->world;
    V2 cellScreenPos = cell->position + state->world->position;
    real32 scale     = 50.0f;

    cellScreenPos *= scale;
    cellScreenPos.y = buffer->height - cellScreenPos.y;

    int32 minX = RoundReal32ToInt32((cellScreenPos.x - world->metrics.innerRadius * scale));
    int32 maxX = RoundReal32ToInt32((cellScreenPos.x + world->metrics.innerRadius * scale));
    int32 minY = RoundReal32ToInt32((cellScreenPos.y - world->metrics.outerRadius * scale));
    int32 maxY = RoundReal32ToInt32((cellScreenPos.y + world->metrics.outerRadius * scale));

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

    real32 v = world->metrics.outerRadius * scale / 2;
    real32 h = world->metrics.innerRadius * scale;

    uint8 *destRow = (uint8 *)buffer->memory + minX * buffer->bytesPerPixel + minY * buffer->pitch;
    for (int32 y = minY; y < maxY; ++y)
    {
        uint32 *dest = (uint32 *)destRow;

        for (int32 x = minX; x < maxX; ++x)
        {
            real32 q2x = Abs(x - cellScreenPos.x);
            real32 q2y = Abs(y - cellScreenPos.y);

            if (2 * v * h - v * q2x - h * q2y >= 0)
            {
                uint32 color = (RoundReal32ToUint32(cell->color.r * 255.0f) << 16) |
                               (RoundReal32ToUint32(cell->color.g * 255.0f) << 8) |
                               (RoundReal32ToUint32(cell->color.b * 255.0f) << 0);

                *dest = color;
            }

            ++dest;
        }

        destRow += buffer->pitch;
    }
}
extern "C" GAME_UPDATE_AND_RENDER(gameUpdateAndRender)
{
    Assert(sizeof(GameState) <= memory->permanentStorageSize);

    GameState *gameState = (GameState *)memory->permanentStorage;
    if (!memory->isInitialized)
    {
        InitializeArena(&gameState->worldArena, memory->permanentStorageSize - sizeof(GameState),
                        (uint8 *)memory->permanentStorage + sizeof(GameState));

        gameState->world = PushStruct(&gameState->worldArena, World);

        World *world               = gameState->world;
        world->metrics.outerRadius = 1.0f;
        world->metrics.innerRadius = world->metrics.outerRadius * 0.866025404f;

        world->metrics.corners[0] = {0.0f, world->metrics.outerRadius};
        world->metrics.corners[1] = {world->metrics.innerRadius, 0.5f * world->metrics.outerRadius};
        world->metrics.corners[2] = {world->metrics.innerRadius,
                                     -0.5f * world->metrics.outerRadius};
        world->metrics.corners[3] = {0.0f, -world->metrics.outerRadius};
        world->metrics.corners[4] = {-world->metrics.innerRadius,
                                     -0.5f * world->metrics.outerRadius};
        world->metrics.corners[5] = {-world->metrics.innerRadius,
                                     0.5f * world->metrics.outerRadius};

        world->position = {1.0f, 1.0f};
        world->width    = 8;
        world->height   = 5;
        world->cells    = PushArray(&gameState->worldArena, world->width * world->height, Cell);

        Cell *cell = world->cells;

        uint32 counter = 0;
        Color c[3]     = {{0.25f, 0.25f, 0.25f}, {0.5f, 0.5f, 0.5f}, {0.75f, 0.75f, 0.75f}};

        for (uint32 y = 0; y < world->height; y++)
        {
            for (uint32 x = 0; x < world->width; x++)
            {
                uint32 colorIndex = counter % 3;
                cell->coord       = HexCoordFromOffsetCoord(x - y / 2, y);
                cell->color       = c[colorIndex];
                cell->position    = {(x + y * 0.5f - y / 2) * world->metrics.innerRadius * 2.0f,
                                     y * world->metrics.outerRadius * 1.5f};

                cell++;
                counter++;
            }
        }

        memory->isInitialized = true;
    }

    Cell *cell = gameState->world->cells;
    for (uint32 i = 0; i < gameState->world->width * gameState->world->height; i++)
    {
        DrawCell(buffer, gameState, cell);

        cell++;
    }
}

extern "C" GAME_GET_SOUND_SAMPLES(gameGetSoundSamples)
{
    GameState *gameState = (GameState *)memory->permanentStorage;
    GameOutputSound(gameState, soundBuffer, 400);
}
