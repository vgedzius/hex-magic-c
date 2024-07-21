#include <SDL2/SDL.h>
#include "hex.h"

HexCoord HexCoordFromOffsetCoord(int x, int y)
{
    HexCoord result;

    result.x = x;
    result.y = y;
    result.z = -x - y;

    return result;
}

void DrawCell(OffScreenBuffer *buffer, GameState *state, Cell *cell)
{
    Camera *camera = &state->camera;
    Vector cellScreenPos = camera->pos - cell->pos + state->grid.pos;

    float cellScreenX = cellScreenPos.x * state->camera.metersToPixels;
    float cellScreenY = cellScreenPos.y * state->camera.metersToPixels;

    float innerRadiusInPixels = state->metrics.innerRadius * state->camera.metersToPixels;
    float outterRadiusInPixels = state->metrics.outerRadius * state->camera.metersToPixels;

    float v = outterRadiusInPixels / 2;
    float h = innerRadiusInPixels;

    int minX = Round(cellScreenX - innerRadiusInPixels);
    int maxX = Round(cellScreenX + innerRadiusInPixels);
    int minY = Round(cellScreenY - outterRadiusInPixels);
    int maxY = Round(cellScreenY + outterRadiusInPixels);

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

    int8_t *destRow = (int8_t *)buffer->pixels + minX * buffer->bytesPerPixel + minY * buffer->pitch;
    for (int y = minY; y < maxY; ++y)
    {
        uint32_t *dest = (uint32_t *)destRow;

        for (int x = minX; x < maxX; ++x)
        {
            float q2x = Abs(x - cellScreenX);
            float q2y = Abs(y - cellScreenY);

            if (2 * v * h - v * q2x - h * q2y >= 0)
            {
                *dest = 0x0000FFFF;
            }

            ++dest;
        }

        destRow += buffer->pitch;
    }
}

void InitGame(GameState *state, int width, int height)
{
    state->metrics.outerRadius = 1.0f;
    state->metrics.innerRadius = state->metrics.outerRadius * 0.866025404f;

    state->metrics.corners[0] = {0.0f, state->metrics.outerRadius};
    state->metrics.corners[1] = {state->metrics.innerRadius, 0.5f * state->metrics.outerRadius};
    state->metrics.corners[2] = {state->metrics.innerRadius, -0.5f * state->metrics.outerRadius};
    state->metrics.corners[3] = {0.0f, -state->metrics.outerRadius};
    state->metrics.corners[4] = {-state->metrics.innerRadius, -0.5f * state->metrics.outerRadius};
    state->metrics.corners[5] = {-state->metrics.innerRadius, 0.5f * state->metrics.outerRadius};

    state->camera.pos = {5.f, 5.f};
    state->camera.speed = 1.0f;
    state->camera.width = width;
    state->camera.height = height;
    state->camera.metersToPixels = 100;

    state->grid.pos = {1.0f, 1.0f};
    state->grid.width = 8;
    state->grid.height = 4;
    state->grid.cells = (Cell *)calloc(state->grid.width * state->grid.height, sizeof(Cell));

    Cell *cell = state->grid.cells;
    for (int y = 0; y < state->grid.height; y++)
    {
        for (int x = 0; x < state->grid.width; x++)
        {
            cell->coord = HexCoordFromOffsetCoord(x - y / 2, y);
            cell->color = {127, 127, 127, 255};
            cell->pos = {(x + y * 0.5f - y / 2) * state->metrics.innerRadius * 2.0f,
                         y * state->metrics.outerRadius * 1.5f};

            cell++;
        }
    }
}

void UpdateGame(OffScreenBuffer *buffer, GameInput *input, GameState *state)
{
    Camera *camera = &state->camera;

    Vector cameraDir = input->arrow * camera->speed;
    cameraDir.x *= -1.0f;
    state->camera.pos += cameraDir;

    Cell *cell = state->grid.cells;
    for (int i = 0; i < state->grid.width * state->grid.height; i++)
    {
        DrawCell(buffer, state, cell);

        cell++;
    }
}