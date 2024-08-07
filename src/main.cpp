#include <SDL2/SDL.h>

#include "hex.h"
#include "hex_platform.h"

int main(int, char **)
{
    if (SDL_Init(SDL_INIT_VIDEO))
    {
        printf("Failed to init SDL: %s\n", SDL_GetError());

        return EXIT_FAILURE;
    }

    GameState state;

    int width = 1200;
    int height = 1000;

    SDL_Window *window = SDL_CreateWindow(
        "Hex Magic",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        width,
        height,
        SDL_WINDOW_ALLOW_HIGHDPI);

    if (window == NULL)
    {
        printf("Could not create window: %s", SDL_GetError());
        return EXIT_FAILURE;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_TARGETTEXTURE);

    if (!renderer)
    {
        printf("Could not create renderer: %s", SDL_GetError());
        return EXIT_FAILURE;
    }

    // const char *fontPath = "assets/fonts/montserrat/Montserrat-Regular.ttf";

    Uint64 now = 0;
    Uint64 thisSecond = 0;
    Uint64 framesThisSecond = 0;

    bool isRunning = true;

    OffScreenBuffer buffer;

    buffer.width = width;
    buffer.height = height;
    buffer.bytesPerPixel = 4;
    buffer.pitch = buffer.width * buffer.bytesPerPixel;
    buffer.pixels = malloc(width * height * buffer.bytesPerPixel);

    Texture renderTexture;
    renderTexture.width = width;
    renderTexture.height = height;
    renderTexture.texture = SDL_CreateTexture(renderer,
                                              SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
                                              width, height);

    InitGame(&state, width, height);

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

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        ClearOffScreenBuffer(&buffer);

        UpdateGame(&buffer, &input, &state);

        SDL_Rect renderRect = {0, 0, width, height};
        SDL_UpdateTexture(renderTexture.texture, &renderRect, buffer.pixels, buffer.width * buffer.bytesPerPixel);
        SDL_RenderCopy(renderer, renderTexture.texture, NULL, &renderRect);

        SDL_RenderPresent(renderer);

        now = SDL_GetTicks64();

        if (now - thisSecond >= 1000)
        {
            printf("FPS: %i\n", (int)framesThisSecond);
            framesThisSecond = 0;
            thisSecond = now;
        }

        framesThisSecond++;
    }

    free(state.grid.cells);
    free(buffer.pixels);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return EXIT_SUCCESS;
}
