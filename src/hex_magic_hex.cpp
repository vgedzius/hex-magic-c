#include "hex_magic_platform.h"
#include "hex_magic_hex.h"
#include "hex_magic_intrinsics.h"

internal HexCoord HexFromOffset(OffsetCoord coord)
{
    HexCoord result;
    int32 parity = coord.y & 1;

    result.q = coord.x - (coord.y - parity) / 2;
    result.r = coord.y;
    result.s = -result.q - result.r;

    return result;
}

internal OffsetCoord OffsetFromHex(HexCoord hex)
{
    OffsetCoord result;
    int32 parity = hex.r & 1;

    result.x = hex.q + (hex.r - parity) / 2;
    result.y = hex.r;

    return result;
}

internal HexCoord RoundHex(HexCoordF hex)
{
    int32 q = RoundReal32ToInt32(hex.q);
    int32 r = RoundReal32ToInt32(hex.r);
    int32 s = RoundReal32ToInt32(hex.s);

    real32 qDiff = Abs(q - hex.q);
    real32 rDiff = Abs(r - hex.r);
    real32 sDiff = Abs(s - hex.s);

    if (qDiff > rDiff && qDiff > sDiff)
    {
        q = -r - s;
    }
    else if (rDiff > sDiff)
    {
        r = -q - s;
    }
    else
    {
        s = -q - r;
    }

    return HexCoord{q, r, s};
}

internal HexCoord V2ToHex(V2 pos)
{
    HexCoordF result;

    result.q = Sqrt(3) / 3.0f * pos.x - 1.0f / 3.0f * pos.y;
    result.r = 2.0f / 3.0f * pos.y;
    result.s = -result.q - result.r;

    return RoundHex(result);
}

internal V2 HexToV2(HexCoord hex)
{
    V2 result;
    real32 sqrt3 = Sqrt(3);

    result.x = sqrt3 * (real32)hex.q + sqrt3 / 2.0f * (real32)hex.r;
    result.y = 1.5f * (real32)hex.r;

    return result;
}

inline bool32 operator==(HexCoord a, HexCoord b)
{
    bool32 result = a.q == b.q && a.r == b.r && a.s == b.s;

    return result;
}

inline bool32 operator!=(HexCoord a, HexCoord b)
{
    bool32 result = !(a == b);

    return result;
}
