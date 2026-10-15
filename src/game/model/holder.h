// Model holders (flags +0, model +4, slot table +0x28: +0 flags, +4 slots of 0x14 bytes,
// +8 count): reimplementation of 0x45bdc0, 0x45bed0-0x45bf70 and 0x4587b0.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x45bdc0: flag 1 (+0), set while the holder is awake (Animator_Play wakes it).
u32 __cdecl Holder_IsAwake(const u8 *holder);

// 0x45bed0: sets flag 0x200 in the model's +0x30 (the holder reached through *ref).
void __cdecl Holder_FlagModel(const u8 *const *ref);

// 0x45bef0: the model's matrix (+0x78, from +0x10) translation (+0x30) to x, y, z.
void __cdecl Holder_SetPosition(const u8 *holder, u32 x, u32 y, u32 z);

// 0x45bf20: the model's matrix (+0x78, from +0x10) axes scaled by scale's x, y, z
// (Matrix_ScaleAxes).
void __cdecl Holder_SetScale(const u8 *holder, const float *scale);

// 0x45bf50, 0x45bf70: the unit's unused statics, 0x7a1cac = 1.0f and 0x7a1cdc = 1 / 2^23.
void __cdecl Holder_InitUnusedOne();
void __cdecl Holder_InitUnusedEpsilon();

// 0x4587b0: slot i's +8 of ride index's holder (table 0x7a4610 -> +8), or 0 when the slot's
// address comes to null.
u32 __cdecl Ride_SlotKey(u32 ride, u32 i);
}
