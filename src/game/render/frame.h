// Per-frame render globals: reimplementation of 0x44e570-0x44e5fb.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x44e570: the bounds at 0x7b0600 (min x, y, z then max x, y, z) to FLT_MAX and -FLT_MAX.
void __cdecl Frame_ResetBounds();

// 0x44e5b0: 0x79e884 |= flags.
void __cdecl Frame_AddFlags(u32 flags);

// 0x44e5d0: copies point (3 dwords, when given) to 0x79e248 and clears 0x79dcd4.
void __cdecl Frame_SetPoint(const u32 *point);
}
