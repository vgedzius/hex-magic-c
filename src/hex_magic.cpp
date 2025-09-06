#include "hex_magic.h"
#include "hex_magic_intrinsics.h"
#include "hex_magic_platform.h"

internal void DrawRectangle(GameOffscreenBuffer *buffer, V2 vMin, V2 vMax, Color c)
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

    uint32 color = (RoundReal32ToUint32(c.r * 255.0f) << 16) |
                   (RoundReal32ToUint32(c.g * 255.0f) << 8) |
                   (RoundReal32ToUint32(c.b * 255.0f) << 0);

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

internal void DrawCell(GameOffscreenBuffer *buffer, World *world, V2 pos, Color color)
{
    int32 minX = RoundReal32ToInt32((pos.x - world->metrics.innerRadius * world->scale));
    int32 maxX = RoundReal32ToInt32((pos.x + world->metrics.innerRadius * world->scale));
    int32 minY = RoundReal32ToInt32((pos.y - world->metrics.outerRadius * world->scale));
    int32 maxY = RoundReal32ToInt32((pos.y + world->metrics.outerRadius * world->scale));

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

    real32 v = world->metrics.outerRadius * world->scale / 2;
    real32 h = world->metrics.innerRadius * world->scale;

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
                uint32 cellColor = (RoundReal32ToUint32(color.r * 255.0f) << 16) |
                                   (RoundReal32ToUint32(color.g * 255.0f) << 8) |
                                   (RoundReal32ToUint32(color.b * 255.0f) << 0);

                *dest = cellColor;
            }

            ++dest;
        }

        destRow += buffer->pitch;
    }
}

inline Cell *GetCellByOffsetCoords(World *world, uint32 x, uint32 y)
{
    Assert(x <= world->width && y <= world->height);

    return &world->cells[y * world->width + x];
}

extern "C" GAME_UPDATE_AND_RENDER(gameUpdateAndRender)
{
    Assert(sizeof(GameState) <= memory->permanentStorageSize);

    GameState *gameState = (GameState *)memory->permanentStorage;
    if (!memory->isInitialized)
    {
        InitializeArena(&gameState->worldArena, memory->permanentStorageSize - sizeof(GameState),
                        (uint8 *)memory->permanentStorage + sizeof(GameState));

        gameState->cameraPos = {8.5f, 4.0f};
        gameState->world     = PushStruct(&gameState->worldArena, World);

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

        world->position = {};
        world->width    = 1000;
        world->height   = 800;
        world->scale    = 50.0f;
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

    real32 cameraMoveSpeed = 5.0f * input->dtForFrame;

    for (uint32 controllerIndex = 0; controllerIndex < ArrayCount(input->controllers);
         ++controllerIndex)
    {
        GameControllerInput *controller = GetController(input, controllerIndex);

        if (controller->moveDown.endedDown)
        {
            gameState->cameraPos.y -= cameraMoveSpeed;
        }

        if (controller->moveUp.endedDown)
        {
            gameState->cameraPos.y += cameraMoveSpeed;
        }

        if (controller->moveLeft.endedDown)
        {
            gameState->cameraPos.x -= cameraMoveSpeed;
        }

        if (controller->moveRight.endedDown)
        {
            gameState->cameraPos.x += cameraMoveSpeed;
        }
    }

    DrawRectangle(buffer, {0.0f, 0.0f}, {(real32)buffer->width, (real32)buffer->height},
                  {1.0f, 0.0f, 1.0f});

    World *world    = gameState->world;
    V2 screenCenter = {0.5f * (real32)buffer->width, 0.5f * (real32)buffer->height};

    for (uint32 y = 0; y < 8; ++y)
    {
        for (uint32 x = 0; x < 12; ++x)
        {
            Cell *cell = GetCellByOffsetCoords(gameState->world, x, y);

            V2 worldPos  = cell->position + world->position;
            V2 screenPos = worldPos - gameState->cameraPos;

            screenPos *= world->scale;
            screenPos += screenCenter;
            screenPos.y = buffer->height - screenPos.y;

            DrawCell(buffer, world, screenPos, cell->color);

            cell++;
        }
    }
}

extern "C" GAME_GET_SOUND_SAMPLES(gameGetSoundSamples)
{
    GameState *gameState = (GameState *)memory->permanentStorage;
    GameOutputSound(gameState, soundBuffer, 400);
}
