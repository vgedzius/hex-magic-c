#if !defined(HEX_MAGIC_HEX_H)

#include "hex_magic_platform.h"
#include "hex_magic_math.h"

struct HexCoord
{
    int32 q, r, s;
};

struct HexCoordF
{
    real32 q, r, s;
};

struct OffsetCoord
{
    int32 x, y;
};

#define HEX_MAGIC_HEX_H
#endif
