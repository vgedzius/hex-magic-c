#ifndef HEX_H_
#define HEX_H_

#include <SDL2/SDL.h>

#include "math.h"

struct HexMetrics
{
    float outerRadius = 1.0f;
    float innerRadius = outerRadius * 0.866025404f;

    Vector corners[6] = {
        {0.0f, outerRadius},
        {innerRadius, 0.5f * outerRadius},
        {innerRadius, -0.5f * outerRadius},
        {0.0f, -outerRadius},
        {-innerRadius, -0.5f * outerRadius},
        {-innerRadius, 0.5f * outerRadius},
    };
};

struct HexCoord
{
    int x, y, z;
};

struct Texture
{
    SDL_Texture *texture;
    int width;
    int height;
};

struct CellUI
{
    Texture xLabel;
    Texture yLabel;
    Texture zLabel;
};

struct Cell
{
    Vector pos;
    HexCoord coord;
    SDL_Color color;
    CellUI ui;
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
};

struct GameInput
{
    Vector arrow;
};

struct GameUi
{
    Texture fps;
};

void UpdateGameUi(SDL_Renderer *renderer, GameUi *ui, TTF_Font *font, int fps);

HexCoord HexCoordFromOffsetCoord(int x, int y);

char *IntToString(int coord);

Texture CreateCoordLabel(SDL_Renderer *renderer, int coord, TTF_Font *font, SDL_Color color);

void InitCellUI(SDL_Renderer *renderer, Cell *cell, TTF_Font *font);

#endif