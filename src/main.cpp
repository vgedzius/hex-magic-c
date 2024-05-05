#include <SDL.h>

int main(int, char **)
{
    if (SDL_Init(SDL_INIT_VIDEO))
    {
        printf("Failed to init SDL: %s\n", SDL_GetError());

        return EXIT_FAILURE;
    }

    int width = 1200;
    int height = 800;

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

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    if (!renderer)
    {
        printf("Could not create renderer: %s", SDL_GetError());
        return EXIT_FAILURE;
    }

    bool isRunning = true;

    while (isRunning)
    {
        SDL_Event event = {0};
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

            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);

            SDL_Rect rect;

            rect.x = 100;
            rect.y = 100;
            rect.w = 50;
            rect.h = 50;

            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);

            SDL_RenderDrawRect(renderer, &rect);

            SDL_RenderPresent(renderer);
        }
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return EXIT_SUCCESS;
}
