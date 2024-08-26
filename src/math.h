#ifndef MATH_H_
#define MATH_H_

#include <math.h>

struct Vector
{
    float x, y;
};

struct Matrix3x3
{
    float a, b, c;
    float d, e, f;
    float g, h, i;
};

struct Matrix1x3
{
    float x, y, w;
};

inline Matrix1x3 operator*(Matrix3x3 a, Matrix1x3 b)
{
    Matrix1x3 result;

    result.x = a.a * b.x + a.b * b.y + a.c * b.w;
    result.y = a.d * b.x + a.e * b.y + a.f * b.w;
    result.w = a.g * b.x + a.h * b.y + a.i * b.w;

    return result;
}

inline Matrix3x3 operator*(Matrix3x3 a, Matrix3x3 b)
{
    Matrix3x3 result;

    result.a = a.a * b.a + a.b * b.d + a.c * b.g;
    result.b = a.a * b.b + a.b * b.e + a.c * b.h;
    result.c = a.a * b.c + a.b * b.f + a.c * b.i;

    result.d = a.d * b.a + a.e * b.d + a.f * b.g;
    result.e = a.d * b.b + a.e * b.e + a.f * b.h;
    result.f = a.d * b.c + a.e * b.f + a.f * b.i;

    result.g = a.g * b.a + a.h * b.d + a.i * b.g;
    result.h = a.g * b.b + a.h * b.e + a.i * b.h;
    result.i = a.g * b.c + a.h * b.f + a.i * b.i;

    return result;
}

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

inline int Abs(int number)
{
    return abs(number);
}

inline int Round(float number)
{
    return (int)roundf(number);
}

#endif