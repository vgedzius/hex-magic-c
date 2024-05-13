#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL2_gfxPrimitives.h>

struct Vector
{
    float x, y;
};

inline Vector operator*(float a, Vector b)
{
    Vector result;

    result.x = a * b.x;
    result.y = a * b.y;

    return result;
}

inline Vector operator*(Vector b, float a)
{
    Vector result = a * b;

    return result;
}

inline Vector &operator*=(Vector &b, float a)
{
    b = a * b;

    return b;
}

inline Vector operator-(Vector a)
{
    Vector result;

    result.x = -a.x;
    result.y = -a.y;

    return result;
}

inline Vector operator+(Vector a, Vector b)
{
    Vector result;

    result.x = a.x + b.x;
    result.y = a.y + b.y;

    return result;
}

inline Vector &operator+=(Vector &a, Vector b)
{
    a = a + b;

    return a;
}

inline Vector operator-(Vector a, Vector b)
{
    Vector result;

    result.x = a.x - b.x;
    result.y = a.y - b.y;

    return result;
}

struct HexMetrics
{
    float outerRadius = 100.0f;
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

struct CoordLabel
{
    SDL_Texture *label;
    int width;
    int height;
};

struct CellUI
{
    CoordLabel xLabel;
    CoordLabel yLabel;
    CoordLabel zLabel;
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

CoordLabel CreateCoordLabel(SDL_Renderer *renderer, int coord, TTF_Font *font, SDL_Color color)
{
    CoordLabel result;
    char *labelText = IntToString(coord);

    SDL_Surface *surface = TTF_RenderUTF8_Blended(font, labelText, color);
    result.label = SDL_CreateTextureFromSurface(renderer, surface);

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

int main(int, char **)
{
    if (SDL_Init(SDL_INIT_VIDEO))
    {
        printf("Failed to init SDL: %s\n", SDL_GetError());

        return EXIT_FAILURE;
    }

    Camera camera;
    camera.pos = {0, 0};
    camera.speed = 100.0f;
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

    Uint64 now = 0;
    Uint64 lastFrame = 0;

    bool isRunning = true;
    bool showCoords = true;

    Vector gridPos = {100, 100};

    Vector xLabelPos = {-60, -25};
    Vector yLabelPos = {15, -65};
    Vector zLabelPos = {5, 15};

    Grid grid;
    grid.width = 56;
    grid.height = 16;
    grid.cells = (Cell *)malloc(grid.width * grid.height * sizeof(Cell));

    Cell *cell = grid.cells;
    for (int y = 0; y < grid.height; y++)
    {
        for (int x = 0; x < grid.width; x++)
        {
            Vector pos = {(x + y * 0.5f - y / 2) * metrics.innerRadius * 2.0f,
                          y * metrics.outerRadius * 1.5f};

            cell->pos = pos + gridPos;
            cell->coord = HexCoordFromOffsetCoord(x - y / 2, y);
            cell->color = {127, 127, 127, 255};

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
                Vector cellScreenPos = cell->pos + camera.pos;

                Vector v0 = cellScreenPos + metrics.corners[0];
                Vector v1 = cellScreenPos + metrics.corners[1];
                Vector v2 = cellScreenPos + metrics.corners[2];
                Vector v3 = cellScreenPos + metrics.corners[3];
                Vector v4 = cellScreenPos + metrics.corners[4];
                Vector v5 = cellScreenPos + metrics.corners[5];

                Sint16 vx[6] = {(Sint16)v0.x, (Sint16)v1.x, (Sint16)v2.x, (Sint16)v3.x, (Sint16)v4.x, (Sint16)v5.x};
                Sint16 vy[6] = {(Sint16)v0.y, (Sint16)v1.y, (Sint16)v2.y, (Sint16)v3.y, (Sint16)v4.y, (Sint16)v5.y};

                filledPolygonRGBA(renderer, vx, vy, 6,
                                  cell->color.r, cell->color.g, cell->color.b, cell->color.a);

                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

                SDL_RenderDrawLineF(renderer, v0.x, v0.y, v1.x, v1.y);
                SDL_RenderDrawLineF(renderer, v1.x, v1.y, v2.x, v2.y);
                SDL_RenderDrawLineF(renderer, v2.x, v2.y, v3.x, v3.y);
                SDL_RenderDrawLineF(renderer, v3.x, v3.y, v4.x, v4.y);
                SDL_RenderDrawLineF(renderer, v4.x, v4.y, v5.x, v5.y);
                SDL_RenderDrawLineF(renderer, v5.x, v5.y, v0.x, v0.y);

                if (showCoords)
                {
                    Vector xPos = cellScreenPos + xLabelPos;
                    Vector yPos = cellScreenPos + yLabelPos;
                    Vector zPos = cellScreenPos + zLabelPos;

                    CoordLabel xLabel = cell->ui.xLabel;
                    CoordLabel yLabel = cell->ui.yLabel;
                    CoordLabel zLabel = cell->ui.zLabel;

                    SDL_Rect xRect = {(int)xPos.x, (int)xPos.y, xLabel.width, xLabel.height};
                    SDL_Rect yRect = {(int)yPos.x, (int)yPos.y, yLabel.width, yLabel.height};
                    SDL_Rect zRect = {(int)zPos.x, (int)zPos.y, zLabel.width, zLabel.height};

                    SDL_RenderCopy(renderer, xLabel.label, NULL, &xRect);
                    SDL_RenderCopy(renderer, yLabel.label, NULL, &yRect);
                    SDL_RenderCopy(renderer, zLabel.label, NULL, &zRect);
                }

                cell++;
            }
        }

        SDL_RenderPresent(renderer);

        now = SDL_GetTicks64();
        Uint64 msThisFrame = now - lastFrame;
        double fps = 1.0f / msThisFrame * 1000.0f;

        lastFrame = now;

        printf("FPS: %f\n", fps);
    }

    free(grid.cells);

    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return EXIT_SUCCESS;
}
