#if !defined(HEX_MAGIC_INTRINSICS)

#include <cmath>
#include <math.h>
#include "hex_magic_platform.h"

inline int32 RoundReal32ToInt32(real32 real32)
{
    int32 result = (int32)roundf(real32);

    return result;
}

inline uint32 RoundReal32ToUint32(real32 real32)
{
    uint32 result = (uint32)roundf(real32);

    return result;
}

inline uint32 FloorReal32ToUint32(real32 real32)
{
    uint32 result = (uint32)floorf(real32);

    return result;
}

inline int32 FloorReal32ToInt32(real32 real32)
{
    int32 result = (int32)floorf(real32);

    return result;
}

inline int32 CeilReal32ToInt32(real32 real32)
{
    int32 result = (int32)ceilf(real32);

    return result;
}

inline uint32 TruncateReal32ToUint32(real32 real32)
{
    uint32 result = (uint32)real32;

    return result;
}

inline real32 Sin(real32 angle)
{
    real32 result = sinf(angle);
    return result;
}

inline real32 Cos(real32 angle)
{
    real32 result = cosf(angle);
    return result;
}

inline real32 Atan2(real32 y, real32 x)
{
    real32 result = atan2f(y, x);
    return result;
}

inline real32 Sqrt(real32 v)
{
    real32 result = sqrtf(v);
    return result;
}

struct BitScanResult
{
    bool32 found;
    uint32 index;
};

inline BitScanResult FindLeastSignificantSetBit(uint32 value)
{
    BitScanResult result = {};

#if COMPILER_GCC
    result.index = __builtin_ctz(value);
    result.found = value != 0;
#else
    for (uint32 test = 0; test < 32; ++test)
    {
        if (value & (1 << test))
        {
            result.index = test;
            result.found = true;
            break;
        }
    }
#endif

    return result;
}

#define HEX_MAGIC_INTRINSICS
#endif
