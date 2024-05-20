#ifndef HEX_H_
#define HEX_H_

#include <SDL2/SDL.h>

#include "math.h"

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

struct Texture
{
    SDL_Texture *texture;
    int width;
    int height;
};

struct UILabel
{
    Vector pos;
    Texture texture;
};

struct CellUI
{
    UILabel xLabel;
    UILabel yLabel;
    UILabel zLabel;
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
    int metersToPixels;
};

struct GameInput
{
    Vector arrow;
};

struct GameUi
{
    Texture fps;
    bool showCoords;
};

struct GameState
{
    GameUi ui;
    Grid grid;
    Camera camera;
    HexMetrics metrics;
};

void UpdateGameUi(SDL_Renderer *renderer, GameUi *ui, TTF_Font *font, int fps);

HexCoord HexCoordFromOffsetCoord(int x, int y);

char *IntToString(int coord);

Texture CreateCoordLabel(SDL_Renderer *renderer, int coord, TTF_Font *font, SDL_Color color);

void InitCellUI(SDL_Renderer *renderer, Cell *cell, TTF_Font *font);

void InitGame(SDL_Renderer *renderer, GameState *state, TTF_Font *font, int width, int height);

void UpdateGame(SDL_Renderer *renderer, GameInput *input, GameState *state);

#endif