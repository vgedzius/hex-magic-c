#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "hex.h"

int main(int, char **)
{
    if (SDL_Init(SDL_INIT_VIDEO))
    {
        printf("Failed to init SDL: %s\n", SDL_GetError());

        return EXIT_FAILURE;
    }

    GameState state;

    state.camera.pos = {0, 0};
    state.camera.speed = 1.0f;
    state.camera.width = 1200;
    state.camera.height = 1000;
    state.camera.metersToPixels = 100;

    state.ui.showCoords = true;

    HexMetrics metrics;

    SDL_Window *window = SDL_CreateWindow(
        "Hex Magic",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        state.camera.width,
        state.camera.height,
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

    UpdateGameUi(renderer, &state.ui, font, 0);

    Uint64 now = 0;
    Uint64 thisSecond = 0;
    Uint64 framesThisSecond = 0;

    bool isRunning = true;

    InitGame(renderer, &state, &metrics, font);

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
                    state.ui.showCoords = !state.ui.showCoords;
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

        UpdateGame(renderer, &input, &state, &metrics);

        SDL_RenderPresent(renderer);

        now = SDL_GetTicks64();

        if (now - thisSecond >= 1000)
        {
            UpdateGameUi(renderer, &state.ui, font, framesThisSecond);
            framesThisSecond = 0;
            thisSecond = now;
        }

        framesThisSecond++;
    }

    free(state.grid.cells);

    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return EXIT_SUCCESS;
}
