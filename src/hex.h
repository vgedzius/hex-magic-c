#ifndef HEX_H_
#define HEX_H_

#include "math.h"
#include "hex_platform.h"

struct HexMetrics
{
    float outerRadius;
    float innerRadius;

    Vector corners[6];
};

struct HexCoord
{
    int x, y, z;
};

struct Cell
{
    Vector pos;
    HexCoord coord;
    SDL_Color color;
};

struct Grid
{
    Vector pos;
    int width;
    int height;

    Cell *cells;
};

struct Camera
{
    Vector pos;
    float speed;
    int width, height;
    int metersToPixels;
};

struct GameInput
{
    Vector arrow;
};

struct GameState
{
    Grid grid;
    Camera camera;
    HexMetrics metrics;
};

HexCoord HexCoordFromOffsetCoord(int x, int y);

void InitGame(GameState *state, int width, int height);

void UpdateGame(OffScreenBuffer *buffer, GameInput *input, GameState *state);

#endif