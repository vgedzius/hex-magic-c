#if !defined HEX_MAGIC_MATH_H
#include <cmath>
#include <climits>

#include "hex_magic_platform.h"

union V2
{
    struct
    {
        real32 x, y;
    };
    real32 e[2];
};

union V3
{
    struct
    {
        real32 x, y, z;
    };
    struct
    {
        real32 r, g, b;
    };
    real32 e[3];
};

inline V2 operator*(real32 a, V2 b)
{
    V2 result;

    result.x = a * b.x;
    result.y = a * b.y;

    return result;
}

inline V2 operator*(V2 a, real32 b)
{
    V2 result = b * a;
    return result;
}

inline V2 &operator*=(V2 &a, real32 b)
{
    a = b * a;
    return a;
}
inline V2 operator-(V2 a)
{
    V2 result;

    result.x = -a.x;
    result.y = -a.y;

    return result;
}

inline V2 operator+(V2 a, V2 b)
{
    V2 result;

    result.x = a.x + b.x;
    result.y = a.y + b.y;

    return result;
}

inline V2 &operator+=(V2 &a, V2 b)
{
    a = a + b;
    return a;
}

inline V2 operator-(V2 a, V2 b)
{
    V2 result;

    result.x = a.x - b.x;
    result.y = a.y - b.y;

    return result;
}

inline V2 &operator-=(V2 &a, V2 b)
{
    a = a - b;
    return a;
}

inline V3 operator*(real32 a, V3 b)
{
    V3 result;

    result.x = a * b.x;
    result.y = a * b.y;
    result.z = a * b.z;

    return result;
}

inline V3 operator*(V3 a, real32 b)
{
    V3 result = b * a;
    return result;
}

inline V3 &operator*=(V3 &a, real32 b)
{
    a = b * a;
    return a;
}
inline V3 operator-(V3 a)
{
    V3 result;

    result.x = -a.x;
    result.y = -a.y;
    result.z = -a.z;

    return result;
}

inline V3 operator+(V3 a, V3 b)
{
    V3 result;

    result.x = a.x + b.x;
    result.y = a.y + b.y;
    result.z = a.z + b.z;

    return result;
}

inline V3 &operator+=(V3 &a, V3 b)
{
    a = a + b;
    return a;
}

inline V3 operator-(V3 a, V3 b)
{
    V3 result;

    result.x = a.x - b.x;
    result.y = a.y - b.y;
    result.z = a.z - b.z;

    return result;
}

inline V3 &operator-=(V3 &a, V3 b)
{
    a = a - b;
    return a;
}

inline uint32 Abs(int32 v)
{
    int32 mask    = v >> sizeof(int32) * (CHAR_BIT - 1);
    uint32 result = (v + mask) ^ mask;

    return result;
}

inline real32 Square(real32 v)
{
    real32 result = v * v;
    return result;
}

inline real32 SquareRoot(real32 v)
{
    real32 result = sqrtf(v);
    return result;
}

inline real32 Inner(V2 a, V2 b)
{
    real32 result = a.x * b.x + a.y * b.y;
    return result;
}

inline real32 LengthSq(V2 v)
{
    real32 result = Inner(v, v);
    return result;
}

inline int32 Min(int32 a, int32 b)
{
    int32 result = a < b ? a : b;
    return result;
}

inline int32 Max(int32 a, int32 b)
{
    int32 result = a > b ? a : b;
    return result;
}

inline V3 Lerp(V3 a, V3 b, real32 t)
{
    V3 result = a + ((b - a) * t);
    return result;
}

#define HEX_MAGIC_MATH_H
#endif
