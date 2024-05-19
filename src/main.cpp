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

    Camera camera;
    camera.pos = {0, 0};
    camera.speed = 1.0f;
    camera.width = 1200;
    camera.height = 1000;

    HexMetrics metrics;

    SDL_Window *window = SDL_CreateWindow(
        "Hex Magic",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        camera.width,
        camera.height,
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

    GameUi ui;
    UpdateGameUi(renderer, &ui, font, 0);

    Uint64 now = 0;
    Uint64 thisSecond = 0;
    Uint64 framesThisSecond = 0;

    bool isRunning = true;
    bool showCoords = true;

    Vector xLabelPos = {-0.6f, -0.25f};
    Vector yLabelPos = {0.15f, -0.65f};
    Vector zLabelPos = {0.05f, 0.15f};

    int metersToPixels = 100;

    Grid grid;
    grid.pos = {1.0f, 1.0f};
    grid.width = 56;
    grid.height = 16;
    grid.cells = (Cell *)malloc(grid.width * grid.height * sizeof(Cell));

    Cell *cell = grid.cells;
    for (int y = 0; y < grid.height; y++)
    {
        for (int x = 0; x < grid.width; x++)
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
                    showCoords = !showCoords;
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

        Vector cameraDir = input.arrow * camera.speed;
        cameraDir.x *= -1.0f;
        camera.pos += cameraDir;

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        Cell *cell = grid.cells;
        for (int y = 0; y < grid.height; y++)
        {
            for (int x = 0; x < grid.width; x++)
            {
                Vector cellScreenPos = grid.pos + cell->pos + camera.pos;

                Vector v0 = cellScreenPos + metrics.corners[0];
                Vector v1 = cellScreenPos + metrics.corners[1];
                Vector v2 = cellScreenPos + metrics.corners[2];
                Vector v3 = cellScreenPos + metrics.corners[3];
                Vector v4 = cellScreenPos + metrics.corners[4];
                Vector v5 = cellScreenPos + metrics.corners[5];

                Sint16 vx[6] = {(Sint16)(v0.x * metersToPixels), (Sint16)(v1.x * metersToPixels),
                                (Sint16)(v2.x * metersToPixels), (Sint16)(v3.x * metersToPixels),
                                (Sint16)(v4.x * metersToPixels), (Sint16)(v5.x * metersToPixels)};

                Sint16 vy[6] = {(Sint16)(v0.y * metersToPixels), (Sint16)(v1.y * metersToPixels),
                                (Sint16)(v2.y * metersToPixels), (Sint16)(v3.y * metersToPixels),
                                (Sint16)(v4.y * metersToPixels), (Sint16)(v5.y * metersToPixels)};

                filledPolygonRGBA(renderer, vx, vy, 6,
                                  cell->color.r, cell->color.g, cell->color.b, cell->color.a);

                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

                SDL_RenderDrawLineF(renderer, v0.x * metersToPixels, v0.y * metersToPixels, v1.x * metersToPixels, v1.y * metersToPixels);
                SDL_RenderDrawLineF(renderer, v1.x * metersToPixels, v1.y * metersToPixels, v2.x * metersToPixels, v2.y * metersToPixels);
                SDL_RenderDrawLineF(renderer, v2.x * metersToPixels, v2.y * metersToPixels, v3.x * metersToPixels, v3.y * metersToPixels);
                SDL_RenderDrawLineF(renderer, v3.x * metersToPixels, v3.y * metersToPixels, v4.x * metersToPixels, v4.y * metersToPixels);
                SDL_RenderDrawLineF(renderer, v4.x * metersToPixels, v4.y * metersToPixels, v5.x * metersToPixels, v5.y * metersToPixels);
                SDL_RenderDrawLineF(renderer, v5.x * metersToPixels, v5.y * metersToPixels, v0.x * metersToPixels, v0.y * metersToPixels);

                if (showCoords)
                {
                    Vector xPos = cellScreenPos + xLabelPos;
                    Vector yPos = cellScreenPos + yLabelPos;
                    Vector zPos = cellScreenPos + zLabelPos;

                    Texture xLabel = cell->ui.xLabel;
                    Texture yLabel = cell->ui.yLabel;
                    Texture zLabel = cell->ui.zLabel;

                    SDL_FRect xRect = {xPos.x * metersToPixels, xPos.y * metersToPixels, (float)xLabel.width, (float)xLabel.height};
                    SDL_FRect yRect = {yPos.x * metersToPixels, yPos.y * metersToPixels, (float)yLabel.width, (float)yLabel.height};
                    SDL_FRect zRect = {zPos.x * metersToPixels, zPos.y * metersToPixels, (float)zLabel.width, (float)zLabel.height};

                    SDL_RenderCopyF(renderer, xLabel.texture, NULL, &xRect);
                    SDL_RenderCopyF(renderer, yLabel.texture, NULL, &yRect);
                    SDL_RenderCopyF(renderer, zLabel.texture, NULL, &zRect);
                }

                cell++;
            }
        }

        SDL_FRect rect = {0, 0, (float)ui.fps.width, (float)ui.fps.height};
        SDL_RenderCopyF(renderer, ui.fps.texture, NULL, &rect);

        SDL_RenderPresent(renderer);

        now = SDL_GetTicks64();

        if (now - thisSecond >= 1000)
        {
            UpdateGameUi(renderer, &ui, font, framesThisSecond);
            framesThisSecond = 0;
            thisSecond = now;
        }

        framesThisSecond++;
    }

    free(grid.cells);

    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return EXIT_SUCCESS;
}
