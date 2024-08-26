#ifndef HEX_H_
#define HEX_H_

#include "math.h"
#include "hex_platform.h"

const int MAX_CHILDREN = 1000;

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

struct Color
{
    float r, g, b;
};

struct Transform
{
    Transform *parent;

    int numberOfChildren = 0;
    Transform *children[MAX_CHILDREN]; // FIXME this should be dynamic, a linked list?

    Vector localPosition;

    bool isDirty = true;
    Matrix3x3 localToWorld;

    bool isInverseDirty;
    Matrix3x3 worldToLocal;
};

struct Cell
{
    Transform transform;
    HexCoord coord;
    Color color;
};

struct Grid
{
    Transform transform;
    int width;
    int height;

    Cell *cells;
};

struct Camera
{
    Transform transform;
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

void DrawCell(OffScreenBuffer *buffer, GameState *state, Cell *cell);

void InitGame(GameState *state, int width, int height);

void UpdateGame(OffScreenBuffer *buffer, GameInput *input, GameState *state);

#endif