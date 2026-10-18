// The integer sine table of 512 steps a turn (0x877358, the u32 just past it 0x877b58):
// reimplementation of its lookups 0x5428a0 and 0x5428c0, which return the entry times 4, and
// of its builder 0x5442e0.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x5428a0 / 0x5428c0 (angle, 512 a turn): entry angle & 0x1ff / (angle + 0x80) & 0x1ff, a
// quarter turn on, shifted left 2.
s32 __cdecl IntSin(u32 angle);
s32 __cdecl IntCos(u32 angle);

// 0x5442e0 (cdecl; name unconfirmed): entry i (0-0x1ff) = __ftol(fsin(2i * 0.00613591796875) * 256.0) (low half; x87 at
// the game's 24-bit precision, the product with the double rounded once).
void __cdecl IntSin_BuildTable();
}
