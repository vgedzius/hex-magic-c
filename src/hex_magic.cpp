#include <cstdio>

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

internal void DrawCell(GameOffscreenBuffer *buffer, World *world, V2 pos, Color color)
{
    real32 sqrt3 = Sqrt(3);

    int32 minX = RoundReal32ToInt32(pos.x - sqrt3 / 2.0f * world->scale);
    int32 maxX = RoundReal32ToInt32(pos.x + sqrt3 / 2.0f * world->scale);
    int32 minY = RoundReal32ToInt32(pos.y - world->scale);
    int32 maxY = RoundReal32ToInt32(pos.y + world->scale);

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

    real32 v = world->scale / 2;
    real32 h = sqrt3 / 2 * world->scale;

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

inline HexCoord HexFromOffset(OffsetCoord coord)
{
    HexCoord result;
    int32 parity = coord.y & 1;

    result.q = coord.x - (coord.y - parity) / 2;
    result.r = coord.y;
    result.s = -result.q - result.r;

    return result;
}

inline OffsetCoord OffsetFromHex(HexCoord hex)
{
    OffsetCoord result;
    int32 parity = hex.r & 1;

    result.x = hex.q + (hex.r - parity) / 2;
    result.y = hex.r;

    return result;
}

inline HexCoord RoundHex(HexCoordF hex)
{
    int32 q = RoundReal32ToInt32(hex.q);
    int32 r = RoundReal32ToInt32(hex.r);
    int32 s = RoundReal32ToInt32(hex.s);

    real32 qDiff = Abs(q - hex.q);
    real32 rDiff = Abs(r - hex.r);
    real32 sDiff = Abs(s - hex.s);

    if (qDiff > rDiff && qDiff > sDiff)
    {
        q = -r - s;
    }
    else if (rDiff > sDiff)
    {
        r = -q - s;
    }
    else
    {
        s = -q - r;
    }

    return HexCoord{q, r, s};
}

inline HexCoord V2ToHex(V2 pos)
{
    HexCoordF result;

    result.q = Sqrt(3) / 3.0f * pos.x - 1.0f / 3.0f * pos.y;
    result.r = 2.0f / 3.0f * pos.y;
    result.s = -result.q - result.r;

    return RoundHex(result);
}

inline V2 HexToV2(HexCoord hex)
{
    V2 result;
    real32 sqrt3 = Sqrt(3);

    result.x = sqrt3 * (real32)hex.q + sqrt3 / 2.0f * (real32)hex.r;
    result.y = 1.5f * (real32)hex.r;

    return result;
}

inline Cell *GetCellByOffset(World *world, OffsetCoord coord)
{
    if (coord.x < 0 || coord.x >= (int32)world->width || coord.y < 0 ||
        coord.y >= (int32)world->height)
    {
        return 0;
    }

    return &world->cells[coord.y * world->width + coord.x];
}

inline V2 ScreenToWorld(GameOffscreenBuffer *buffer, GameState *state, uint32 x, uint32 y)
{
    V2 result;
    V2 screenCenter = {0.5f * (real32)buffer->width, 0.5f * (real32)buffer->height};
    World *world    = state->world;

    result.x = (real32)x;
    result.y = (real32)buffer->height - (real32)y;

    result -= screenCenter;
    result *= 1.0f / world->scale;
    result += state->camera.position;

    return result;
}

inline bool operator==(HexCoord a, HexCoord b) { return a.q == b.q && a.r == b.r && a.s == b.s; }

inline bool operator!=(HexCoord a, HexCoord b) { return !(a == b); }

extern "C" GAME_UPDATE_AND_RENDER(gameUpdateAndRender)
{
    Assert(sizeof(GameState) <= memory->permanentStorageSize);

    GameState *gameState = (GameState *)memory->permanentStorage;
    if (!memory->isInitialized)
    {
        InitializeArena(&gameState->worldArena, memory->permanentStorageSize - sizeof(GameState),
                        (uint8 *)memory->permanentStorage + sizeof(GameState));

        gameState->camera.position = {8.5f, 4.0f};
        gameState->camera.velocity = {};
        gameState->camera.speed    = 100.0f;

        gameState->world = PushStruct(&gameState->worldArena, World);

        World *world        = gameState->world;
        world->position     = {};
        world->width        = 900;
        world->height       = 600;
        world->scale        = 50.0f;
        world->selectedCell = 0;
        world->cells        = PushArray(&gameState->worldArena, world->width * world->height, Cell);

        Cell *cell = world->cells;

        uint32 counter = 0;
        Color c[3]     = {{0.25f, 0.25f, 0.25f}, {0.5f, 0.5f, 0.5f}, {0.75f, 0.75f, 0.75f}};

        for (int32 y = 0; y < world->height; y++)
        {
            for (int32 x = 0; x < world->width; x++)
            {
                cell->coord    = HexFromOffset({x, y});
                cell->color    = c[(y & 1 ? counter : counter + 1) % 3];
                cell->position = HexToV2(cell->coord);

                cell++;
                counter++;
            }
        }

        memory->isInitialized = true;
    }

    int32 mouseControlZone = 50;

    GameControllerInput *controller = GetController(input, 0);
    V2 ddCamera                     = {};

    printf("ended down? %d\n", controller->moveUp.endedDown);

    if (controller->moveDown.endedDown || input->mouseY > buffer->height - mouseControlZone)
    {
        ddCamera.y = -1.0f;
    }

    if (controller->moveUp.endedDown || input->mouseY < mouseControlZone)
    {
        ddCamera.y = 1.0f;
    }

    if (controller->moveLeft.endedDown || input->mouseX < mouseControlZone)
    {
        ddCamera.x = -1.0f;
    }

    if (controller->moveRight.endedDown || input->mouseX > buffer->width - mouseControlZone)
    {
        ddCamera.x = 1.0f;
    }

    if ((ddCamera.x != 0.0f) && (ddCamera.y != 0.0f))
    {
        ddCamera *= 0.707106781187f;
    }

    Camera *camera = &gameState->camera;

    ddCamera *= camera->speed;
    ddCamera += -10.0f * camera->velocity;

    camera->position = 0.5f * ddCamera * Square(input->dtForFrame) +
                       camera->velocity * input->dtForFrame + camera->position;
    camera->velocity = ddCamera * input->dtForFrame + camera->velocity;

    DrawRectangle(buffer, {0.0f, 0.0f}, {(real32)buffer->width, (real32)buffer->height},
                  {1.0f, 0.0f, 1.0f});

    World *world             = gameState->world;
    V2 screenCenter          = {0.5f * (real32)buffer->width, 0.5f * (real32)buffer->height};
    HexCoord cameraHexPos    = V2ToHex(gameState->camera.position);
    OffsetCoord cameraOffset = OffsetFromHex(cameraHexPos);

    V2 mouseWorldPos     = ScreenToWorld(buffer, gameState, input->mouseX, input->mouseY);
    HexCoord mouseHexPos = V2ToHex(mouseWorldPos);

    Color hoverColor    = {1.0f, 1.0f, 1.0f};
    Color selectedColor = {1.0f, 0.0f, 0.0f};

    if (input->mouseButtons[0].endedDown)
    {
        world->selectedCell = GetCellByOffset(world, OffsetFromHex(mouseHexPos));
    }

    for (int32 relY = -5; relY < 5; ++relY)
    {
        for (int32 relX = -8; relX < 8; ++relX)
        {
            int32 x = cameraOffset.x + relX;
            int32 y = cameraOffset.y + relY;

            Cell *cell = GetCellByOffset(gameState->world, {x, y});
            if (cell)
            {
                V2 cellWorldPos  = cell->position + world->position;
                V2 cellScreenPos = cellWorldPos - gameState->camera.position;

                cellScreenPos *= world->scale;
                cellScreenPos += screenCenter;
                cellScreenPos.y = buffer->height - cellScreenPos.y;

                Color color;
                if (world->selectedCell && world->selectedCell->coord == cell->coord)
                {
                    color = selectedColor;
                }
                else if (mouseHexPos == cell->coord)
                {
                    color = hoverColor;
                }
                else
                {
                    color = cell->color;
                }

                DrawCell(buffer, world, cellScreenPos, color);
            }
        }
    }
}

extern "C" GAME_GET_SOUND_SAMPLES(gameGetSoundSamples)
{
    GameState *gameState = (GameState *)memory->permanentStorage;
    GameOutputSound(gameState, soundBuffer, 400);
}
