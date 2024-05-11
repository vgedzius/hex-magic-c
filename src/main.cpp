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
    SDL_Rect rect;
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

CoordLabel CreateCoordLabel(SDL_Renderer *renderer, int coord, Vector pos, TTF_Font *font, SDL_Color color)
{
    char *labelText = IntToString(coord);
    int width, height;
    CoordLabel result;

    SDL_Surface *surface = TTF_RenderUTF8_Blended(font, labelText, color);
    result.label = SDL_CreateTextureFromSurface(renderer, surface);

    TTF_SizeUTF8(font, labelText, &width, &height);

    result.rect.x = pos.x;
    result.rect.y = pos.y;
    result.rect.w = width;
    result.rect.h = height;

    free(labelText);

    return result;
}

void InitCellUI(SDL_Renderer *renderer, Cell *cell, TTF_Font *font)
{
    SDL_Color xlabelColor = {255, 0, 0};
    SDL_Color ylabelColor = {0, 255, 0};
    SDL_Color zlabelColor = {0, 0, 255};

    Vector xLabelPos = {-60, -25};
    Vector yLabelPos = {15, -65};
    Vector zLabelPos = {5, 15};

    Vector xPos = cell->pos + xLabelPos;
    Vector yPos = cell->pos + yLabelPos;
    Vector zPos = cell->pos + zLabelPos;

    cell->ui.xLabel = CreateCoordLabel(renderer, cell->coord.x, xPos, font, xlabelColor);
    cell->ui.yLabel = CreateCoordLabel(renderer, cell->coord.y, yPos, font, ylabelColor);
    cell->ui.zLabel = CreateCoordLabel(renderer, cell->coord.z, zPos, font, zlabelColor);
}

void RenderCellUI(SDL_Renderer *renderer, Cell *cell)
{
    SDL_RenderCopy(renderer, cell->ui.xLabel.label, NULL, &cell->ui.xLabel.rect);
    SDL_RenderCopy(renderer, cell->ui.yLabel.label, NULL, &cell->ui.yLabel.rect);
    SDL_RenderCopy(renderer, cell->ui.zLabel.label, NULL, &cell->ui.zLabel.rect);
}

int main(int, char **)
{
    if (SDL_Init(SDL_INIT_VIDEO))
    {
        printf("Failed to init SDL: %s\n", SDL_GetError());

        return EXIT_FAILURE;
    }

    int width = 1200;
    int height = 1000;

    HexMetrics metrics;

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

    const char *fontPath = "assets/fonts/montserrat/Montserrat-Regular.ttf";

    TTF_Init();
    TTF_Font *font = TTF_OpenFont(fontPath, 42);

    if (font == NULL)
    {
        printf("Font not found: %s\n", fontPath);
        return EXIT_FAILURE;
    }

    bool isRunning = true;
    bool showCoords = true;

    Vector gridPos = {100, 100};

    Grid grid;
    grid.width = 6;
    grid.height = 6;
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

            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);

            Cell *cell = grid.cells;
            for (int y = 0; y < grid.height; y++)
            {
                for (int x = 0; x < grid.width; x++)
                {
                    Vector v0 = cell->pos + metrics.corners[0];
                    Vector v1 = cell->pos + metrics.corners[1];
                    Vector v2 = cell->pos + metrics.corners[2];
                    Vector v3 = cell->pos + metrics.corners[3];
                    Vector v4 = cell->pos + metrics.corners[4];
                    Vector v5 = cell->pos + metrics.corners[5];

                    Sint16 vx[6] = {(Sint16)v0.x, (Sint16)v1.x, (Sint16)v2.x, (Sint16)v3.x, (Sint16)v4.x, (Sint16)v5.x};
                    Sint16 vy[6] = {(Sint16)v0.y, (Sint16)v1.y, (Sint16)v2.y, (Sint16)v3.y, (Sint16)v4.y, (Sint16)v5.y};

                    filledPolygonRGBA(renderer, vx, vy, 6,
                                      cell->color.r, cell->color.g, cell->color.b, cell->color.a);

                    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

                    SDL_RenderDrawLine(renderer, v0.x, v0.y, v1.x, v1.y);
                    SDL_RenderDrawLine(renderer, v1.x, v1.y, v2.x, v2.y);
                    SDL_RenderDrawLine(renderer, v2.x, v2.y, v3.x, v3.y);
                    SDL_RenderDrawLine(renderer, v3.x, v3.y, v4.x, v4.y);
                    SDL_RenderDrawLine(renderer, v4.x, v4.y, v5.x, v5.y);
                    SDL_RenderDrawLine(renderer, v5.x, v5.y, v0.x, v0.y);

                    if (showCoords)
                    {
                        RenderCellUI(renderer, cell);
                    }

                    cell++;
                }
            }

            SDL_RenderPresent(renderer);
        }
    }

    free(grid.cells);

    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return EXIT_SUCCESS;
}
