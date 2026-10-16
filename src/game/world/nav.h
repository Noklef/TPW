// A moving person's steering and pathing state (Nav, the sprite's +0xd4, serialized at 0x50d150):
// reimplementation of 0x5100e0 and 0x510160 (thiscall). +4 radius, +8 pos and +0x10 vel (each an
// x, y pair, 16.16 fixed point), +0x18 max_force, +0x1c max_speed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x5100e0 (x, y, vx, vy): pos and vel set.
void __fastcall Nav_SetPosVel(u8 *nav, void *, u32 x, u32 y, u32 vx, u32 vy);

// 0x510160 (&x, &y): pos read (x then y).
void __fastcall Nav_GetPos(const u8 *nav, void *, u32 *x, u32 *y);
}
