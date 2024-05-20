#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL2_gfxPrimitives.h>

#include "hex.h"

int main(int, char **)
{
    if (SDL_Init(SDL_INIT_VIDEO))
    {
        printf("Failed to init SDL: %s\n", SDL_GetError());

        return EXIT_FAILURE;
    }

    GameState *state = (GameState *)malloc(sizeof(GameState));

    state->camera.pos = {0, 0};
    state->camera.speed = 1.0f;
    state->camera.width = 1200;
    state->camera.height = 1000;
    state->camera.metersToPixels = 100;

    state->ui.showCoords = true;

    HexMetrics metrics;

    SDL_Window *window = SDL_CreateWindow(
        "Hex Magic",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        state->camera.width,
        state->camera.height,
        SDL_WINDOW_ALLOW_HIGHDPI);

    if (window == NULL)
    {
        printf("Could not create window: %s", SDL_GetError());
        return EXIT_FAILURE;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    if (!renderer)
    {
        printf("Could not create renderer: %s", SDL_GetError());
        return EXIT_FAILURE;
    }

    const char *fontPath = "assets/fonts/montserrat/Montserrat-Regular.ttf";

    TTF_Init();
    TTF_Font *font = TTF_OpenFont(fontPath, 42);

    if (font == NULL)
    {
        printf("Font not found: %s\n", fontPath);
        return EXIT_FAILURE;
    }

    UpdateGameUi(renderer, &state->ui, font, 0);

    Uint64 now = 0;
    Uint64 thisSecond = 0;
    Uint64 framesThisSecond = 0;

    bool isRunning = true;

    Vector xLabelPos = {-0.6f, -0.25f};
    Vector yLabelPos = {0.15f, -0.65f};
    Vector zLabelPos = {0.05f, 0.15f};

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
            cell->pos = {(x + y * 0.5f - y / 2) * metrics.innerRadius * 2.0f,
                         y * metrics.outerRadius * 1.5f};

            InitCellUI(renderer, cell, font);

            cell++;
        }
    }

    while (isRunning)
    {
        SDL_Event event = {0};
        GameInput input = {0};

        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                isRunning = false;
            }

            if (event.type == SDL_KEYUP)
            {
                SDL_Keycode key = event.key.keysym.sym;

                if (key == SDLK_q || key == SDLK_ESCAPE)
                {
                    isRunning = false;
                }

                if (key == SDLK_c)
                {
                    state->ui.showCoords = !state->ui.showCoords;
                }
            }

            if (event.type == SDL_KEYDOWN)
            {
                SDL_Keycode key = event.key.keysym.sym;

                if (key == SDLK_RIGHT)
                {
                    input.arrow.x = 1.0f;
                }

                if (key == SDLK_LEFT)
                {
                    input.arrow.x = -1.0f;
                }

                if (key == SDLK_UP)
                {
                    input.arrow.y = 1.0f;
                }

                if (key == SDLK_DOWN)
                {
                    input.arrow.y = -1.0f;
                }
            }
        }

        Vector cameraDir = input.arrow * state->camera.speed;
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

                Vector v0 = cellScreenPos + metrics.corners[0];
                Vector v1 = cellScreenPos + metrics.corners[1];
                Vector v2 = cellScreenPos + metrics.corners[2];
                Vector v3 = cellScreenPos + metrics.corners[3];
                Vector v4 = cellScreenPos + metrics.corners[4];
                Vector v5 = cellScreenPos + metrics.corners[5];

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
                    Vector xPos = cellScreenPos + xLabelPos;
                    Vector yPos = cellScreenPos + yLabelPos;
                    Vector zPos = cellScreenPos + zLabelPos;

                    Texture xLabel = cell->ui.xLabel;
                    Texture yLabel = cell->ui.yLabel;
                    Texture zLabel = cell->ui.zLabel;

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

        SDL_RenderPresent(renderer);

        now = SDL_GetTicks64();

        if (now - thisSecond >= 1000)
        {
            UpdateGameUi(renderer, &state->ui, font, framesThisSecond);
            framesThisSecond = 0;
            thisSecond = now;
        }

        framesThisSecond++;
    }

    free(state->grid.cells);
    free(state);

    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return EXIT_SUCCESS;
}
