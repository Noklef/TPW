// The staff base class's small methods (the thing's pay grade +0x1e4, happiness +0x1f8,
// energy +0x1fc, name +0x1a0): reimplementation of 0x506680-0x506860.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x506680 (thiscall): whether the energy (+0x1fc), __ftol'd and cut to its low byte, is
// below the rest level ([0x785324], signed).
u32 __fastcall Staff_IsTooTired(const u8 *staff);

// 0x5066a0 (thiscall): energy less (6 - grade) * 0.012 and happiness less (6 - grade) *
// 0.005, each clamped to [0, 100] (above 100 gives 100; below 0, or NaN, gives 0).
void __fastcall Staff_TireA(u8 *staff);

// 0x506760 (thiscall): as Staff_TireA by 0.025f and 0.01; the happiness drain is stored to a
// float before it is added.
void __fastcall Staff_TireB(u8 *staff);

// 0x506830 (thiscall): wcsncpy of 32 units of name to +0x1a0, and the unit after them
// (+0x1e0) zeroed.
void __fastcall Staff_SetName(u8 *staff, void *, const u16 *name);

// 0x506860 (thiscall): the name, +0x1a0.
u16 *__fastcall Staff_GetName(u8 *staff);
}
