#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL2_gfxPrimitives.h>
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

    cell->ui.xLabel.pos = {-0.6f, -0.25f};
    cell->ui.yLabel.pos = {0.15f, -0.65f};
    cell->ui.zLabel.pos = {0.05f, 0.15f};

    cell->ui.xLabel.texture = CreateCoordLabel(renderer, cell->coord.x, font, xlabelColor);
    cell->ui.yLabel.texture = CreateCoordLabel(renderer, cell->coord.y, font, ylabelColor);
    cell->ui.zLabel.texture = CreateCoordLabel(renderer, cell->coord.z, font, zlabelColor);
}

void InitGame(SDL_Renderer *renderer, GameState *state, HexMetrics *metrics, TTF_Font *font)
{
    state->grid.pos = {1.0f, 1.0f};
    state->grid.width = 56;
    state->grid.height = 16;
    state->grid.cells = (Cell *)calloc(state->grid.width * state->grid.height, sizeof(Cell));

    Cell *cell = state->grid.cells;
    for (int y = 0; y < state->grid.height; y++)
    {
        for (int x = 0; x < state->grid.width; x++)
        {
            cell->coord = HexCoordFromOffsetCoord(x - y / 2, y);
            cell->color = {127, 127, 127, 255};
            cell->pos = {(x + y * 0.5f - y / 2) * metrics->innerRadius * 2.0f,
                         y * metrics->outerRadius * 1.5f};

            InitCellUI(renderer, cell, font);

            cell++;
        }
    }
}

void UpdateGame(SDL_Renderer *renderer, GameInput *input, GameState *state, HexMetrics *metrics)
{
    Vector cameraDir = input->arrow * state->camera.speed;
    cameraDir.x *= -1.0f;
    state->camera.pos += cameraDir;

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    Cell *cell = state->grid.cells;
    for (int y = 0; y < state->grid.height; y++)
    {
        for (int x = 0; x < state->grid.width; x++)
        {
            Camera camera = state->camera;
            Vector cellScreenPos = state->grid.pos + cell->pos + camera.pos;

            Vector v0 = cellScreenPos + metrics->corners[0];
            Vector v1 = cellScreenPos + metrics->corners[1];
            Vector v2 = cellScreenPos + metrics->corners[2];
            Vector v3 = cellScreenPos + metrics->corners[3];
            Vector v4 = cellScreenPos + metrics->corners[4];
            Vector v5 = cellScreenPos + metrics->corners[5];

            Sint16 vx[6] = {(Sint16)(v0.x * camera.metersToPixels), (Sint16)(v1.x * camera.metersToPixels),
                            (Sint16)(v2.x * camera.metersToPixels), (Sint16)(v3.x * camera.metersToPixels),
                            (Sint16)(v4.x * camera.metersToPixels), (Sint16)(v5.x * camera.metersToPixels)};

            Sint16 vy[6] = {(Sint16)(v0.y * camera.metersToPixels), (Sint16)(v1.y * camera.metersToPixels),
                            (Sint16)(v2.y * camera.metersToPixels), (Sint16)(v3.y * camera.metersToPixels),
                            (Sint16)(v4.y * camera.metersToPixels), (Sint16)(v5.y * camera.metersToPixels)};

            filledPolygonRGBA(renderer, vx, vy, 6,
                              cell->color.r, cell->color.g, cell->color.b, cell->color.a);

            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

            SDL_RenderDrawLineF(renderer, v0.x * camera.metersToPixels, v0.y * camera.metersToPixels, v1.x * camera.metersToPixels, v1.y * camera.metersToPixels);
            SDL_RenderDrawLineF(renderer, v1.x * camera.metersToPixels, v1.y * camera.metersToPixels, v2.x * camera.metersToPixels, v2.y * camera.metersToPixels);
            SDL_RenderDrawLineF(renderer, v2.x * camera.metersToPixels, v2.y * camera.metersToPixels, v3.x * camera.metersToPixels, v3.y * camera.metersToPixels);
            SDL_RenderDrawLineF(renderer, v3.x * camera.metersToPixels, v3.y * camera.metersToPixels, v4.x * camera.metersToPixels, v4.y * camera.metersToPixels);
            SDL_RenderDrawLineF(renderer, v4.x * camera.metersToPixels, v4.y * camera.metersToPixels, v5.x * camera.metersToPixels, v5.y * camera.metersToPixels);
            SDL_RenderDrawLineF(renderer, v5.x * camera.metersToPixels, v5.y * camera.metersToPixels, v0.x * camera.metersToPixels, v0.y * camera.metersToPixels);

            if (state->ui.showCoords)
            {
                Vector xPos = cellScreenPos + cell->ui.xLabel.pos;
                Vector yPos = cellScreenPos + cell->ui.yLabel.pos;
                Vector zPos = cellScreenPos + cell->ui.zLabel.pos;

                Texture xLabel = cell->ui.xLabel.texture;
                Texture yLabel = cell->ui.yLabel.texture;
                Texture zLabel = cell->ui.zLabel.texture;

                SDL_FRect xRect = {xPos.x * camera.metersToPixels, xPos.y * camera.metersToPixels, (float)xLabel.width, (float)xLabel.height};
                SDL_FRect yRect = {yPos.x * camera.metersToPixels, yPos.y * camera.metersToPixels, (float)yLabel.width, (float)yLabel.height};
                SDL_FRect zRect = {zPos.x * camera.metersToPixels, zPos.y * camera.metersToPixels, (float)zLabel.width, (float)zLabel.height};

                SDL_RenderCopyF(renderer, xLabel.texture, NULL, &xRect);
                SDL_RenderCopyF(renderer, yLabel.texture, NULL, &yRect);
                SDL_RenderCopyF(renderer, zLabel.texture, NULL, &zRect);
            }

            cell++;
        }
    }

    SDL_FRect rect = {0, 0, (float)state->ui.fps.width, (float)state->ui.fps.height};
    SDL_RenderCopyF(renderer, state->ui.fps.texture, NULL, &rect);
}