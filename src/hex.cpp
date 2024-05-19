#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "hex.h"

void UpdateGameUi(SDL_Renderer *renderer, GameUi *ui, TTF_Font *font, int fps)
{
    SDL_Color color = {255, 255, 255, 255};

    char text[20]; // TODO: buffer overflow???
    sprintf(text, "FPS: %i", fps);

    SDL_Surface *surface = TTF_RenderUTF8_Blended(font, text, color);
    ui->fps.texture = SDL_CreateTextureFromSurface(renderer, surface);

    TTF_SizeUTF8(font, text, &ui->fps.width, &ui->fps.height);
}

HexCoord HexCoordFromOffsetCoord(int x, int y)
{
    HexCoord result;

    result.x = x;
    result.y = y;
    result.z = -x - y;

    return result;
}

char *IntToString(int coord)
{
    const char *format = "%i";

    size_t needed = snprintf(NULL, 0, format, coord) + 1;
    char *buffer = (char *)malloc(needed);

    sprintf(buffer, format, coord);

    return buffer;
}

Texture CreateCoordLabel(SDL_Renderer *renderer, int coord, TTF_Font *font, SDL_Color color)
{
    Texture result;
    char *labelText = IntToString(coord);

    SDL_Surface *surface = TTF_RenderUTF8_Blended(font, labelText, color);
    result.texture = SDL_CreateTextureFromSurface(renderer, surface);

    TTF_SizeUTF8(font, labelText, &result.width, &result.height);

    free(labelText);

    return result;
}

void InitCellUI(SDL_Renderer *renderer, Cell *cell, TTF_Font *font)
{
    SDL_Color xlabelColor = {255, 0, 0};
    SDL_Color ylabelColor = {0, 255, 0};
    SDL_Color zlabelColor = {0, 0, 255};

    cell->ui.xLabel = CreateCoordLabel(renderer, cell->coord.x, font, xlabelColor);
    cell->ui.yLabel = CreateCoordLabel(renderer, cell->coord.y, font, ylabelColor);
    cell->ui.zLabel = CreateCoordLabel(renderer, cell->coord.z, font, zlabelColor);
}