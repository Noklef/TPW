// The park's random number generator: reimplementation of 0x516330 and 0x516370. The
// state is one word in the world object (gWorld, 0x7cf83c) at +0x1da708; the park's
// simulation draws from it, so it must stay bit-exact for saved games and replays to
// play out the same.
#pragma once

#include "../../../re/types.h"

extern "C" {
// 0x516330 (__thiscall on the world): state = ror(state * 0x19660d + 0x3c6ef35f, 13);
// returns its absolute value (0x80000000 stays negative).
s32 __fastcall World_Rand(u8 *world);
// 0x516370 (__thiscall; seed): the state.
void __fastcall World_SetSeed(u8 *world, void *edx, u32 seed);
}
