// The hand-written angle routine in cseg (0x6e7074): the direction of a vector in 0x800 steps a turn, from the arctangent table at
// 0x6e71b4 (u16 entries, indexed by the smaller magnitude * 256 / the larger).
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x6e7074 (cdecl (dx, dy); name unconfirmed): 0 for (0, 0). Else the magnitudes (NEG: 0x80000000 stays negative and compares as
// such, sic) by a signed compare; q = (the smaller << 8) / the larger (32-bit, unsigned: a zero divisor faults, sic, as for
// (0, 0x80000000)); the entry t = [0x6e71b4 + 2q] turned by the quadrant: dx >= 0: dy >= 0 0x400 - t (|dx| < |dy|) or 0x200 + t,
// dy < 0 t or 0x200 - t; dx < 0: dy >= 0 0x400 + t or 0x600 - t, dy < 0 0x800 - t or 0x600 + t; & 0x7ff in AX, q's upper half
// above it.
u32 __cdecl IntAngle_Atan2(s32 dx, s32 dy);
}
