#if !defined HEX_MAGIC_MATH
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
    struct
    {
        V2 xy;
        real32 ignored0_;
    };
    struct
    {
        real32 ignored1_;
        V2 yz;
    };
    real32 e[3];
};

union V4
{
    struct
    {
        union
        {
            V3 xyz;
            struct
            {
                real32 x, y, z;
            };
        };
        real32 w;
    };
    struct
    {
        union
        {
            V3 rgb;
            struct
            {
                real32 r, g, b;
            };
        };
        real32 a;
    };
    real32 e[4];
};

inline V2 Vector2(real32 x, real32 y)
{
    V2 result;

    result.x = x;
    result.y = y;

    return result;
}

inline V2 Vector2(int32 x, int32 y)
{
    V2 result = Vector2((real32)x, (real32)y);
    return result;
}

inline V3 Vector3(real32 x, real32 y, real32 z)
{
    V3 result;

    result.x = x;
    result.y = y;
    result.z = z;

    return result;
}

inline V3 Vector3(V2 xy, real32 z)
{
    V3 result;

    result.x = xy.x;
    result.y = xy.y;
    result.z = z;

    return result;
}

inline V4 Vector4(real32 x, real32 y, real32 z, real32 w)
{
    V4 result;

    result.x = x;
    result.y = y;
    result.z = z;
    result.w = w;

    return result;
}

inline V4 Vector4(V3 xyz, real32 w)
{
    V4 result;

    result.xyz = xyz;
    result.w   = w;

    return result;
}

// Scalar operations

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

inline real32 Clamp(real32 min, real32 value, real32 max)
{
    real32 result = value;

    if (result < min)
    {
        result = min;
    }
    else if (result > max)
    {
        result = max;
    }

    return result;
}

inline real32 Clamp01(real32 value)
{
    real32 result = Clamp(0.0f, value, 1.0f);
    return result;
}

// Vector2 operations

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

inline V2 Lerp(V2 a, V2 b, real32 t)
{
    V2 result = a + ((b - a) * t);
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

inline V2 Perp(V2 v)
{
    V2 result = {-v.y, v.x};
    return result;
}

// Vector3 operations

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

inline V3 Lerp(V3 a, V3 b, real32 t)
{
    V3 result = a + ((b - a) * t);
    return result;
}

inline real32 Inner(V3 a, V3 b)
{
    real32 result = a.x * b.x + a.y * b.y + a.z * b.z;
    return result;
}

inline real32 LengthSq(V3 v)
{
    real32 result = Inner(v, v);
    return result;
}

inline V3 Clamp01(V3 value)
{
    V3 result;

    result.x = Clamp01(value.x);
    result.y = Clamp01(value.y);
    result.z = Clamp01(value.z);

    return result;
}

// Vector4 operations

inline V4 operator*(real32 a, V4 b)
{
    V4 result;

    result.x = a * b.x;
    result.y = a * b.y;
    result.z = a * b.z;
    result.w = a * b.w;

    return result;
}

inline V4 operator*(V4 a, real32 b)
{
    V4 result = b * a;
    return result;
}

inline V4 &operator*=(V4 &a, real32 b)
{
    a = b * a;
    return a;
}
inline V4 operator-(V4 a)
{
    V4 result;

    result.x = -a.x;
    result.y = -a.y;
    result.z = -a.z;
    result.w = -a.w;

    return result;
}

inline V4 operator+(V4 a, V4 b)
{
    V4 result;

    result.x = a.x + b.x;
    result.y = a.y + b.y;
    result.z = a.z + b.z;
    result.w = a.w + b.w;

    return result;
}

inline V4 &operator+=(V4 &a, V4 b)
{
    a = a + b;
    return a;
}

inline V4 operator-(V4 a, V4 b)
{
    V4 result;

    result.x = a.x - b.x;
    result.y = a.y - b.y;
    result.z = a.z - b.z;
    result.w = a.w - b.w;

    return result;
}

inline V4 &operator-=(V4 &a, V4 b)
{
    a = a - b;
    return a;
}

inline V4 Lerp(V4 a, V4 b, real32 t)
{
    V4 result = a + ((b - a) * t);
    return result;
}

inline real32 Inner(V4 a, V4 b)
{
    real32 result = a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
    return result;
}

inline real32 LengthSq(V4 v)
{
    real32 result = Inner(v, v);
    return result;
}

inline V4 Hadamard(V4 a, V4 b)
{
    V4 result = {a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w};
    return result;
}

#define HEX_MAGIC_MATH
#endif
