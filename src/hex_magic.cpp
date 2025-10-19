#include <cstdio>
#include <cstring>
#include "hex_magic.h"
#include "hex_magic_hex.h"
#include "hex_magic_intrinsics.h"
#include "hex_magic_math.h"
#include "hex_magic_platform.h"

#include "hex_magic_hex.cpp"
#include "hex_magic_render.cpp"

inline void InitializeArena(MemoryArena *arena, MemoryIndex size, uint8 *base)
{
    arena->size      = size;
    arena->base      = base;
    arena->used      = 0;
    arena->tempCount = 0;
}

inline TemporaryMemory StartTemporaryMemory(MemoryArena *arena)
{
    TemporaryMemory result = {};

    result.arena = arena;
    result.used  = arena->used;

    ++arena->tempCount;

    return result;
}

inline void EndTemporaryMemory(TemporaryMemory memory)
{
    memory.arena->used = memory.used;

    Assert(memory.arena->tempCount > 0);

    --memory.arena->tempCount;
}

inline void CheckArena(MemoryArena *arena) { Assert(arena->tempCount == 0); }

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

internal uint32 AddResource(World *world, V2 position)
{
    uint32 index   = AddEntity(world);
    Entity *entity = GetEntity(world, index);

    entity->type     = ENTITY_RESOURCE;
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

internal void DrawEntity(Renderer *renderer, Camera *camera, V2 position, V2 dimensions, V4 color)
{
    RendererDrawRectangle(renderer, camera->position, position, dimensions, color);
}

internal void DrawResource(Renderer *renderer, Camera *camera, V2 position)
{
    V2 dimensions = {1.25f, 1.25f};
    V4 color      = {0.5f, 0.0f, 0.5f, 1.0f};

    DrawEntity(renderer, camera, position, dimensions, color);
}

internal void DrawCity(Renderer *renderer, Camera *camera, V2 position)
{
    V2 dimensions = {1.0f, 1.0f};
    V4 color      = {0.5f, 0.5f, 0.5f, 1.0f};

    DrawEntity(renderer, camera, position, dimensions, color);
}

internal void DrawHero(Renderer *renderer, Camera *camera, V2 position)
{
    V2 dimensions = {0.5f, 0.5f};
    V4 color      = {1.0f, 1.0f, 0.0f, 1.0f};

    DrawEntity(renderer, camera, position, dimensions, color);
}

internal Cell *GetCell(World *world, OffsetCoord coord)
{
    Cell *result = 0;

    if (coord.x > 0 && coord.x < (int32)world->width && coord.y > 0 && coord.y < (int32)world->height)
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

internal V4 BiomeColor(Biome biome)
{
    V4 result;

    switch (biome)
    {
        case GRASS:
        {
            result = {0.18039215686f, 0.2862745098f, 0.10980392157f, 1.0f};
        }
        break;

        case DIRT:
        {
            result = {0.38431372549f, 0.28235294118f, 0.18039215686f, 1.0f};
        }
        break;

        case LAVA:
        {
            result = {0.20784313725f, 0.18039215686f, 0.18039215686f, 1.0f};
        }
        break;

        case ROUGH:
        {
            result = {0.48235294118f, 0.37254901961f, 0.27450980392f, 1.0f};
        }
        break;

        case SAND:
        {
            result = {0.76470588235f, 0.63137254902f, 0.47450980392f, 1.0f};
        }
        break;

        case SNOW:
        {
            result = {0.92549019608f, 0.93725490196f, 0.93725490196f, 1.0f};
        }
        break;

        case WATER:
        {
            result = {0.06274509804f, 0.17647058824f, 0.30196078431f, 1.0f};
        }
        break;

        case SWAMP:
        {
            result = {0.17254901961f, 0.30980392157f, 0.20784313725f, 1.0f};
        }
        break;

        case ROCK:
        {
            result = {0.36862745098f, 0.19607843137f, 0.07843137255f, 1.0f};
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

    Assert(sizeof(TransientState) <= memory->transientStorageSize);
    TransientState *transientState = (TransientState *)memory->transientStorage;
    if (!transientState->isInitialized)
    {
        InitializeArena(&transientState->transientArena, memory->transientStorageSize - sizeof(TransientState),
                        (uint8 *)memory->transientStorage + sizeof(TransientState));

        transientState->isInitialized = true;
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
            memory->debugPlatformWriteEntireFile(thread, "world.map", gameState->worldArena.size, world);
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

    camera->zoom =
        0.5f * ddCameraZoom * Square(input->dtForFrame) + camera->zoomVelocity * input->dtForFrame + camera->zoom;
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

    camera->position =
        0.5f * ddCamera * Square(input->dtForFrame) + camera->velocity * input->dtForFrame + camera->position;
    camera->velocity = ddCamera * input->dtForFrame + camera->velocity;

    TemporaryMemory renderMemory = StartTemporaryMemory(&transientState->transientArena);
    Renderer *renderer           = MakeRenderer(&transientState->transientArena, Megabytes(4), camera->zoom);

    V4 bgColor = {0.392f, 0.584f, 0.929f, 1.0f};
#if HEX_MAGIC_INTERNAL
    if (gameState->mode == EDIT)
    {
        bgColor = {1.0f, 0.0f, 1.0f, 1.0f};
    }
#endif

    RendererClear(renderer, bgColor);

    HexCoord cameraHexPos    = V2ToHex(camera->position);
    OffsetCoord cameraOffset = OffsetFromHex(cameraHexPos);

    V2 mouseWorldPos     = ScreenToWorld(buffer, camera, mouse->x, mouse->y);
    HexCoord mouseHexPos = V2ToHex(mouseWorldPos);

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

                        case ENTITY_RESOURCE:
                        {
                            if (!cell->resourceIndex)
                            {
                                cell->resourceIndex = AddResource(world, cell->position);
                            }
                        }
                        break;
                    }
                }
            }
        }
    }
#endif

    V4 white           = {1.0f, 1.0f, 1.0f, 1.0f};
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

                V4 color = BiomeColor(cell->biome);

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

                RendererDrawHex(renderer, camera->position, cell->position, color);

                if (cell->resourceIndex)
                {
                    DrawResource(renderer, camera, cell->position);
                }

                if (cell->cityIndex)
                {
                    DrawCity(renderer, camera, cell->position);
                }

                if (cell->heroIndex)
                {
                    DrawHero(renderer, camera, cell->position);
                }

#if HEX_MAGIC_INTERNAL
                if (isHovering && gameState->mode == EDIT && editor->brush == BRUSH_ENTITY)
                {
                    switch (editor->brushEntity)
                    {
                        case ENTITY_RESOURCE:
                        {
                            DrawResource(renderer, camera, cell->position);
                        }
                        break;

                        case ENTITY_CITY:
                        {
                            DrawCity(renderer, camera, cell->position);
                        }
                        break;

                        case ENTITY_HERO:
                        {
                            DrawHero(renderer, camera, cell->position);
                        }
                        break;
                    }
                }
#endif
            }
        }
    }

    RenderToOutput(buffer, renderer);

    EndTemporaryMemory(renderMemory);

    CheckArena(&gameState->worldArena);
    CheckArena(&transientState->transientArena);
}

extern "C" GAME_GET_SOUND_SAMPLES(gameGetSoundSamples)
{
    GameState *gameState = (GameState *)memory->permanentStorage;
    GameOutputSound(gameState, soundBuffer, 400);
}
