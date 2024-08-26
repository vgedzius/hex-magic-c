#include "hex.h"

Vector Zero()
{
    Vector result = {0.0f, 0.0f};

    return result;
}

Matrix3x3 Identity()
{
    Matrix3x3 result = {1.0f, 0.0f, 0.0f,
                        0.0f, 1.0f, 0.0f,
                        0.0f, 0.0f, 1.0f};

    return result;
}

void Translate(Transform *transform, Vector point)
{
    transform->localPosition = transform->localPosition + point;
    SetDirty(transform);
}

Matrix3x3 TranslateMatrix(Vector pos)
{
    Matrix3x3 result = {1.0f, 0.0f, pos.x,
                        0.0f, 1.0f, pos.y,
                        0.0f, 0.0f, 1.0f};

    return result;
}

void SetDirty(Transform *transform)
{
    if (transform->isDirty)
        return;

    transform->isDirty = true;
    transform->isInverseDirty = true;

    for (int i = 0; i < transform->numberOfChildren; i++)
    {
        SetDirty(transform->children[i]);
    }
}

void Remove(Transform *transform, Transform *child)
{
    bool removed = false;
    for (int i = 0; i < transform->numberOfChildren; i++)
    {
    }

    if (removed)
    {
        transform->numberOfChildren--;
    }
}

Matrix3x3 CalculateLocalToParentMatrix(Transform *transform)
{
    return TranslateMatrix(transform->localPosition);
}

Matrix3x3 LocalToWorldMatrix(Transform *transform)
{
    if (transform->isDirty)
    {
        if (transform->parent == NULL)
        {
            transform->localToWorld = CalculateLocalToParentMatrix(transform);
        }
        else
        {
            transform->localToWorld = LocalToWorldMatrix(transform->parent) * CalculateLocalToParentMatrix(transform);
        }

        transform->isDirty = false;
    }

    return transform->localToWorld;
}

Vector TransformPoint(Transform *transform, Vector point)
{
    Matrix1x3 pointMatrix = {point.x, point.y, 1.0f};
    Matrix1x3 transformResult = LocalToWorldMatrix(transform) * pointMatrix;

    Vector result = {transformResult.x, transformResult.y};

    return result;
}

HexCoord HexCoordFromOffsetCoord(int x, int y)
{
    HexCoord result;

    result.x = x;
    result.y = y;
    result.z = -x - y;

    return result;
}

void DrawCell(OffScreenBuffer *buffer, GameState *state, Cell *cell)
{
    Camera *camera = &state->camera;
    Vector cellWorldPosition = TransformPoint(&cell->transform, Zero());
    Vector cameraWorldPosition = TransformPoint(&camera->transform, Zero());
    Vector cellScreenPos = cameraWorldPosition - cellWorldPosition;

    float cellScreenX = cellScreenPos.x * state->camera.metersToPixels;
    float cellScreenY = cellScreenPos.y * state->camera.metersToPixels;

    float innerRadiusInPixels = state->metrics.innerRadius * state->camera.metersToPixels;
    float outterRadiusInPixels = state->metrics.outerRadius * state->camera.metersToPixels;

    float v = outterRadiusInPixels / 2;
    float h = innerRadiusInPixels;

    int minX = Round(cellScreenX - innerRadiusInPixels);
    int maxX = Round(cellScreenX + innerRadiusInPixels);
    int minY = Round(cellScreenY - outterRadiusInPixels);
    int maxY = Round(cellScreenY + outterRadiusInPixels);

    if (minX < 0)
    {
        minX = 0;
    }

    if (maxX > buffer->width)
    {
        maxX = buffer->width;
    }

    if (minY < 0)
    {
        minY = 0;
    }

    if (maxY > buffer->height)
    {
        maxY = buffer->height;
    }

    int8_t *destRow = (int8_t *)buffer->pixels + minX * buffer->bytesPerPixel + minY * buffer->pitch;
    for (int y = minY; y < maxY; ++y)
    {
        uint32_t *dest = (uint32_t *)destRow;

        for (int x = minX; x < maxX; ++x)
        {
            float q2x = Abs(x - cellScreenX);
            float q2y = Abs(y - cellScreenY);

            if (2 * v * h - v * q2x - h * q2y >= 0)
            {
                uint32_t color = (Round(cell->color.r * 255.0f) << 24) |
                                 (Round(cell->color.g * 255.0f) << 16) |
                                 (Round(cell->color.b * 255.0f) << 8);

                *dest = color;
            }

            ++dest;
        }

        destRow += buffer->pitch;
    }
}

void InitGame(GameState *state, int width, int height)
{
    state->metrics.outerRadius = 1.0f;
    state->metrics.innerRadius = state->metrics.outerRadius * 0.866025404f;

    state->metrics.corners[0] = {0.0f, state->metrics.outerRadius};
    state->metrics.corners[1] = {state->metrics.innerRadius, 0.5f * state->metrics.outerRadius};
    state->metrics.corners[2] = {state->metrics.innerRadius, -0.5f * state->metrics.outerRadius};
    state->metrics.corners[3] = {0.0f, -state->metrics.outerRadius};
    state->metrics.corners[4] = {-state->metrics.innerRadius, -0.5f * state->metrics.outerRadius};
    state->metrics.corners[5] = {-state->metrics.innerRadius, 0.5f * state->metrics.outerRadius};

    state->camera.transform.parent = NULL;
    state->camera.transform.localPosition = {5.0f, 5.0f};
    state->camera.speed = 1.0f;
    state->camera.width = width;
    state->camera.height = height;
    state->camera.metersToPixels = 100;

    state->grid.transform.parent = NULL;
    state->grid.transform.localPosition = {1.0f, 1.0f};
    state->grid.width = 8;
    state->grid.height = 4;
    state->grid.cells = (Cell *)calloc(state->grid.width * state->grid.height, sizeof(Cell));

    Cell *cell = state->grid.cells;
    for (int y = 0; y < state->grid.height; y++)
    {
        for (int x = 0; x < state->grid.width; x++)
        {
            cell->coord = HexCoordFromOffsetCoord(x - y / 2, y);
            cell->color = {0.5f, 0.5f, 0.5f};

            cell->transform.parent = &state->grid.transform;
            cell->transform.localPosition = {(x + y * 0.5f - y / 2) * state->metrics.innerRadius * 2.0f,
                                             y * state->metrics.outerRadius * 1.5f};

            SetDirty(&cell->transform);

            cell++;
        }
    }
}

void UpdateGame(OffScreenBuffer *buffer, GameInput *input, GameState *state)
{
    Camera *camera = &state->camera;

    Vector cameraDir = input->arrow * camera->speed;
    cameraDir.x *= -1.0f;
    Translate(&state->camera.transform, cameraDir);

    Cell *cell = state->grid.cells;
    for (int i = 0; i < state->grid.width * state->grid.height; i++)
    {
        DrawCell(buffer, state, cell);

        cell++;
    }
}