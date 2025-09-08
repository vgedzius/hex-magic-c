#include <climits>
#if !defined HEX_MAGIC_MATH_H

#include "hex_magic_platform.h"

union V2
{
    struct
    {
        real32 x, y;
    };
    real32 e[2];
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

inline uint32 Abs(int32 v)
{
    int32 mask    = v >> sizeof(int32) * (CHAR_BIT - 1);
    uint32 result = (v + mask) ^ mask;

    return result;
}

#define HEX_MAGIC_MATH_H
#endif
