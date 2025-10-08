#include <cstdio>

#include <cstring>
#include "hex_magic.h"
#include "hex_magic_hex.h"
#include "hex_magic_intrinsics.h"
#include "hex_magic_math.h"
#include "hex_magic_platform.h"

#include "hex_magic_hex.cpp"

internal void DrawRectangle(GameOffscreenBuffer *buffer, V2 vMin, V2 vMax, V3 c)
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

internal uint32 AddEntity(World *world)
{
    uint32 enitityIndex = ++world->entityCount;
    Assert(enitityIndex < ArrayCount(world->entities));

    return enitityIndex;
}

internal Entity *GetEntity(World *world, uint32 index)
{
    Entity *hero = 0;

    if (index > 0 && index < ArrayCount(world->entities))
    {
        hero = &world->entities[index];
    }

    return hero;
}

internal uint32 AddHero(World *world, V2 position)
{
    uint32 index   = AddEntity(world);
    Entity *entity = GetEntity(world, index);

    entity->type     = ENTITY_HERO;
    entity->position = position;

    return index;
}

internal uint32 AddCity(World *world, V2 position)
{
    uint32 index   = AddEntity(world);
    Entity *entity = GetEntity(world, index);

    entity->type     = ENTITY_CITY;
    entity->position = position;

    return index;
}

internal V2 ScreenToWorld(GameOffscreenBuffer *buffer, Camera *camera, uint32 x, uint32 y)
{
    V2 result;
    V2 screenCenter = {0.5f * (real32)buffer->width, 0.5f * (real32)buffer->height};

    result.x = (real32)x;
    result.y = (real32)buffer->height - (real32)y;

    result -= screenCenter;
    result *= 1.0f / camera->zoom;
    result += camera->position;

    return result;
}

internal V2 PointToScreen(GameOffscreenBuffer *buffer, Camera *camera, V2 point)
{
    V2 screenCenter = {0.5f * (real32)buffer->width, 0.5f * (real32)buffer->height};
    V2 result       = point - camera->position;

    result *= camera->zoom;
    result += screenCenter;
    result.y = buffer->height - result.y;

    return result;
}

internal void DrawCell(GameOffscreenBuffer *buffer, Camera *camera, Cell *cell, V3 color)
{
    real32 sqrt3 = Sqrt(3);
    V2 pos       = PointToScreen(buffer, camera, cell->position);

    int32 minX = RoundReal32ToInt32(pos.x - sqrt3 / 2.0f * camera->zoom);
    int32 maxX = RoundReal32ToInt32(pos.x + sqrt3 / 2.0f * camera->zoom);
    int32 minY = RoundReal32ToInt32(pos.y - camera->zoom);
    int32 maxY = RoundReal32ToInt32(pos.y + camera->zoom);

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

    real32 v = camera->zoom / 2;
    real32 h = sqrt3 / 2 * camera->zoom;

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

internal void DrawEntity(GameOffscreenBuffer *buffer, Camera *camera, V2 position, V2 dimensions,
                         V3 color)
{
    V2 screenPosition = PointToScreen(buffer, camera, position);

    V2 min = {screenPosition.x - dimensions.x * 0.5f * camera->zoom,
              screenPosition.y - dimensions.y * 0.5f * camera->zoom};

    V2 max = {screenPosition.x + dimensions.x * 0.5f * camera->zoom,
              screenPosition.y + dimensions.y * 0.5f * camera->zoom};

    DrawRectangle(buffer, min, max, color);
}

internal void DrawCity(GameOffscreenBuffer *buffer, Camera *camera, V2 position)
{
    V2 dimensions = {1.0f, 1.0f};
    V3 color      = {0.5f, 0.5f, 0.5f};

    DrawEntity(buffer, camera, position, dimensions, color);
}

internal void DrawHero(GameOffscreenBuffer *buffer, Camera *camera, V2 position)
{
    V2 dimensions = {0.5f, 0.5f};
    V3 color      = {1.0f, 1.0f, 0.0f};

    DrawEntity(buffer, camera, position, dimensions, color);
}

internal Cell *GetCell(World *world, OffsetCoord coord)
{
    Cell *result = 0;

    if (coord.x > 0 && coord.x < (int32)world->width && coord.y > 0 &&
        coord.y < (int32)world->height)
    {
        result = &world->cells[coord.y * world->width + coord.x];
    }

    return result;
}

internal Cell *GetCell(World *world, HexCoord coord)
{
    OffsetCoord offsetCoord = OffsetFromHex(coord);
    Cell *result            = GetCell(world, offsetCoord);

    return result;
}

internal V3 BiomeColor(Biome biome)
{
    V3 result;

    switch (biome)
    {
        case GRASS:
        {
            result = {0.18039215686f, 0.2862745098f, 0.10980392157f};
        }
        break;

        case DIRT:
        {
            result = {0.38431372549f, 0.28235294118f, 0.18039215686f};
        }
        break;

        case LAVA:
        {
            result = {0.20784313725f, 0.18039215686f, 0.18039215686f};
        }
        break;

        case ROUGH:
        {
            result = {0.48235294118f, 0.37254901961f, 0.27450980392f};
        }
        break;

        case SAND:
        {
            result = {0.76470588235f, 0.63137254902f, 0.47450980392f};
        }
        break;

        case SNOW:
        {
            result = {0.92549019608f, 0.93725490196f, 0.93725490196f};
        }
        break;

        case WATER:
        {
            result = {0.06274509804f, 0.17647058824f, 0.30196078431f};
        }
        break;

        case SWAMP:
        {
            result = {0.17254901961f, 0.30980392157f, 0.20784313725f};
        }
        break;

        case ROCK:
        {
            result = {0.36862745098f, 0.19607843137f, 0.07843137255f};
        }
        break;
    }

    return result;
}

inline bool32 WasPressed(GameButtonState button)
{
    bool32 result = button.endedDown && button.halfTransitionCount > 0;
    return result;
}

inline bool32 WasReleased(GameButtonState button)
{
    bool32 result = !button.endedDown && button.halfTransitionCount > 0;
    return result;
}

inline bool32 IsHeld(GameButtonState button)
{
    bool32 result = button.endedDown;
    return result;
}

extern "C" GAME_UPDATE_AND_RENDER(gameUpdateAndRender)
{
    Assert(sizeof(GameState) <= memory->permanentStorageSize);

    GameState *gameState = (GameState *)memory->permanentStorage;
    if (!memory->isInitialized)
    {
        InitializeArena(&gameState->worldArena, memory->permanentStorageSize - sizeof(GameState),
                        (uint8 *)memory->permanentStorage + sizeof(GameState));

        gameState->camera.zoom         = 75.0f;
        gameState->camera.zoomVelocity = 0.0f;
        gameState->camera.zoomSpeed    = 7500.0f;
        gameState->camera.zoomFriction = 7.5f;
        gameState->camera.minZoom      = 25.0f;
        gameState->camera.maxZoom      = 150.0f;
        gameState->camera.position     = {8.5f, 4.0f};
        gameState->camera.velocity     = {};
        gameState->camera.speed        = 15.0f;
        gameState->camera.friction     = 10.0f;

        gameState->editor              = {};
        gameState->editor.brushSize    = 0;
        gameState->editor.minBrushSize = 0;
        gameState->editor.maxBrushSize = 5;

        gameState->mode = PLAY;

        gameState->world = PushStruct(&gameState->worldArena, World);
        World *world     = gameState->world;

        world->width        = 160;
        world->height       = 100;
        world->selectedCell = 0;
        world->entityCount  = 0;

        world->cells = PushArray(&gameState->worldArena, world->width * world->height, Cell);

        Cell *cell = world->cells;
        for (int32 y = 0; y < world->height; ++y)
        {
            for (int32 x = 0; x < world->width; ++x)
            {
                cell->coord    = HexFromOffset({x, y});
                cell->biome    = WATER;
                cell->position = HexToV2(cell->coord);

                ++cell;
            }
        }

        AddEntity(world);

        memory->isInitialized = true;
    }

    World *world   = gameState->world;
    Editor *editor = &gameState->editor;

    int32 mouseControlZone = 50;

    GameKeyboardInput *keyboard = &input->keyboard;
    GameMouseInput *mouse       = &input->mouse;
    V2 ddCamera                 = {};

#if HEX_MAGIC_INTERNAL
    if (WasPressed(keyboard->toggleMode))
    {
        gameState->mode     = gameState->mode == EDIT ? PLAY : EDIT;
        world->selectedCell = 0;
    }

    if (gameState->mode == EDIT)
    {
        if (WasPressed(keyboard->nextBiome))
        {
            editor->brush = BRUSH_BIOME;

            // TODO figure out this enum stuff how to loop arround to the beginning.
            if (editor->brushBiome == ROCK)
            {
                editor->brushBiome = (Biome)0;
            }
            else
            {
                editor->brushBiome = (Biome)(editor->brushBiome + 1);
            }
        }

        if (WasPressed(keyboard->nextEntity))
        {
            editor->brush = BRUSH_ENTITY;

            // TODO figure out this enum stuff how to loop arround to the beginning.
            if (editor->brushEntity == ENTITY_CITY)
            {
                editor->brushEntity = (EntityType)0;
            }
            else
            {
                editor->brushEntity = (EntityType)(editor->brushEntity + 1);
            }
        }

        if (WasPressed(keyboard->save))
        {
            memory->debugPlatformWriteEntireFile(thread, "world.map", gameState->worldArena.size,
                                                 world);
        }

        if (WasPressed(keyboard->load))
        {
            DebugReadFileResult result = memory->debugPlatformReadEntireFile(thread, "world.map");
            if (result.contentsSize)
            {
                memcpy(world, result.contents, result.contentsSize);
            }
        }

        if (editor->brush == BRUSH_BIOME)
        {
            if (WasPressed(keyboard->actionUp) && editor->brushSize < editor->maxBrushSize)
            {
                editor->brushSize++;
            }

            if (WasPressed(keyboard->actionDown) && editor->brushSize > editor->minBrushSize)
            {
                editor->brushSize--;
            }
        }
    }
#endif

    if (WasPressed(keyboard->cancel))
    {
        world->selectedCell = 0;
    }

    Camera *camera = &gameState->camera;

    real32 ddCameraZoom = input->mouse.wheel;
    if (input->mouse.wheel > 0 && camera->zoom < camera->maxZoom)
    {
        ddCameraZoom = input->mouse.wheel;
    }
    else if (input->mouse.wheel < 0 && camera->zoom > camera->minZoom)
    {
        ddCameraZoom = input->mouse.wheel;
    }

    ddCameraZoom *= camera->zoomSpeed;
    ddCameraZoom += -camera->zoomFriction * camera->zoomVelocity;

    camera->zoom = 0.5f * ddCameraZoom * Square(input->dtForFrame) +
                   camera->zoomVelocity * input->dtForFrame + camera->zoom;
    camera->zoomVelocity = ddCameraZoom * input->dtForFrame + camera->zoomVelocity;

    if (camera->zoom < camera->minZoom)
    {
        camera->zoom = camera->minZoom;
    }
    if (camera->zoom > camera->maxZoom)
    {
        camera->zoom = camera->maxZoom;
    }

    if (IsHeld(keyboard->moveDown) || mouse->y > buffer->height - mouseControlZone)
    {
        ddCamera.y = -1.0f;
    }

    if (IsHeld(keyboard->moveUp) || mouse->y < mouseControlZone)
    {
        ddCamera.y = 1.0f;
    }

    if (IsHeld(keyboard->moveLeft) || mouse->x < mouseControlZone)
    {
        ddCamera.x = -1.0f;
    }

    if (IsHeld(keyboard->moveRight) || mouse->x > buffer->width - mouseControlZone)
    {
        ddCamera.x = 1.0f;
    }

    real32 ddCameraLength = LengthSq(ddCamera);
    if (ddCameraLength > 1.0f)
    {
        ddCamera *= 1.0f / SquareRoot(ddCameraLength);
    }

    ddCamera *= camera->speed * (1000.0f / camera->zoom);
    ddCamera += -camera->friction * camera->velocity;

    camera->position = 0.5f * ddCamera * Square(input->dtForFrame) +
                       camera->velocity * input->dtForFrame + camera->position;
    camera->velocity = ddCamera * input->dtForFrame + camera->velocity;

    V3 bgColor = {0.392f, 0.584f, 0.929f};
#if HEX_MAGIC_INTERNAL
    if (gameState->mode == EDIT)
    {
        bgColor = {1.0f, 0.0f, 1.0f};
    }
#endif

    DrawRectangle(buffer, {0.0f, 0.0f}, {(real32)buffer->width, (real32)buffer->height}, bgColor);

    HexCoord cameraHexPos    = V2ToHex(camera->position);
    OffsetCoord cameraOffset = OffsetFromHex(cameraHexPos);

    V2 mouseWorldPos     = ScreenToWorld(buffer, camera, mouse->x, mouse->y);
    HexCoord mouseHexPos = V2ToHex(mouseWorldPos);

    V3 white = {1.0f, 1.0f, 1.0f};

    if (WasPressed(mouse->lButton))
    {
        if (gameState->mode == PLAY)
        {
            world->selectedCell = GetCell(world, OffsetFromHex(mouseHexPos));
        }
    }

#if HEX_MAGIC_INTERNAL
    if (gameState->mode == EDIT)
    {
        if (IsHeld(mouse->lButton))
        {
            Cell *cell = GetCell(world, OffsetFromHex(mouseHexPos));
            if (cell)
            {
                if (editor->brush == BRUSH_BIOME)
                {
                    if (editor->brushSize > 0)
                    {
                        int32 n = editor->brushSize;
                        for (int32 q = -n; q <= n; ++q)
                        {
                            for (int32 r = Max(-n, -q - n); r <= Min(n, -q + n); ++r)
                            {
                                HexCoord coord    = cell->coord + HexCoord{q, r, -q - r};
                                Cell *cellToPaint = GetCell(world, coord);

                                if (cellToPaint)
                                {
                                    cellToPaint->biome = editor->brushBiome;
                                }
                            }
                        }
                    }
                    else
                    {
                        cell->biome = editor->brushBiome;
                    }
                }

                if (editor->brush == BRUSH_ENTITY)
                {
                    switch (editor->brushEntity)
                    {
                        case ENTITY_HERO:
                        {
                            if (!cell->heroIndex)
                            {
                                cell->heroIndex = AddHero(world, cell->position);
                            }
                        }
                        break;

                        case ENTITY_CITY:
                        {
                            if (!cell->cityIndex)
                            {
                                cell->cityIndex = AddCity(world, cell->position);
                            }
                        }
                        break;
                    }
                }
            }
        }
    }
#endif

    real32 innerRadius = Sqrt(3) / 2.0f;
    int32 xSpan        = CeilReal32ToInt32(buffer->width / (4 * innerRadius * camera->zoom)) + 2;
    int32 ySpan        = buffer->height / 3;
    for (int32 relY = -ySpan; relY < ySpan; ++relY)
    {
        for (int32 relX = -xSpan; relX < xSpan; ++relX)
        {
            int32 x = cameraOffset.x + relX;
            int32 y = cameraOffset.y + relY;

            Cell *cell = GetCell(gameState->world, OffsetCoord{x, y});
            if (cell)
            {
                bool32 isHovering = mouseHexPos == cell->coord;

#if HEX_MAGIC_INTERNAL
                if (gameState->mode == EDIT && editor->brush == BRUSH_BIOME &&
                    Distance(cell->coord, mouseHexPos) <= editor->brushSize)
                {
                    isHovering = true;
                }
#endif

                V3 color = BiomeColor(cell->biome);

                if (world->selectedCell && world->selectedCell->coord == cell->coord)
                {
                    color = Lerp(color, white, 0.2f);
                }
                else if (isHovering)
                {
#if HEX_MAGIC_INTERNAL
                    if (gameState->mode == EDIT && editor->brush == BRUSH_BIOME)
                    {
                        color = BiomeColor(editor->brushBiome);
                    }
#endif
                    color = Lerp(color, white, 0.1f);
                }

                DrawCell(buffer, camera, cell, color);

                if (cell->cityIndex)
                {
                    Entity *entity = GetEntity(world, cell->cityIndex);
                    DrawCity(buffer, camera, cell->position);
                }

                if (cell->heroIndex)
                {
                    Entity *entity = GetEntity(world, cell->heroIndex);
                    DrawHero(buffer, camera, cell->position);
                }

#if HEX_MAGIC_INTERNAL
                if (isHovering && gameState->mode == EDIT && editor->brush == BRUSH_ENTITY)
                {
                    switch (editor->brushEntity)
                    {
                        case ENTITY_CITY:
                        {
                            DrawCity(buffer, camera, cell->position);
                        }
                        break;

                        case ENTITY_HERO:
                        {
                            DrawHero(buffer, camera, cell->position);
                        }
                        break;
                    }
                }
#endif
            }
        }
    }
}

extern "C" GAME_GET_SOUND_SAMPLES(gameGetSoundSamples)
{
    GameState *gameState = (GameState *)memory->permanentStorage;
    GameOutputSound(gameState, soundBuffer, 400);
}
