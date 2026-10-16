// A graphics level and a coaster's car count: reimplementation of 0x41a9d0 and 0x43ba20. Named by what they do until the callers are known.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x41a9d0 (no arguments): in the 0x4000 render size, 8 / 6 for a 8 / 16 bit depth, 4 for 32 and else 0; otherwise the level 0 to 3 of gBalanceGraphics (0x7858d0) as 2, 4, 6,
// 8, and 8 for any larger.
u32 __cdecl Graphics_Level();

// 0x43ba20 (coaster, count): nothing when count is 0. Otherwise, with a bit of 0xc in the flags at +8, the cars from count up to the current count (+0xf4) are unloaded on both
// sides (CoasterTrain_Unload, the trains of 0x80 bytes from the pointer at +0x128); then +0xf4 is count.
void __cdecl Coaster_SetTrainCount(u8 *coaster, u32 count);

}  // extern "C"
