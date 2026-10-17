// A moving person's steering and pathing state (Nav, the sprite's +0xd4, serialized at 0x50d150):
// reimplementation of 0x5100e0, 0x510160, its constructor 0x50ffe0 and destructor 0x50f200 (thiscall). +4 radius, +8 pos and +0x10 vel (each an
// x, y pair, 16.16 fixed point), +0x18 max_force, +0x1c max_speed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x5100e0 (x, y, vx, vy): pos and vel set.
void __fastcall Nav_SetPosVel(u8 *nav, void *, u32 x, u32 y, u32 vx, u32 vy);

// 0x510160 (&x, &y): pos read (x then y).
void __fastcall Nav_GetPos(const u8 *nav, void *, u32 *x, u32 *y);
// 0x50ffe0 (thiscall): constants and zeroes into the first 0xbc bytes (the final values: +0 0x10000, +4 and +0x1c 0x3333, +0x18 0x6666, the byte at +0x60 1; +0x48 is left
// alone), and the five 8-byte elements at +0x64 constructed (IntPair_Ctor, through CRT_VectorCtorIterator). Returns the object.
u8 *__fastcall Nav_Ctor(u8 *object);

// 0x50f200 (thiscall; also reached by the jumps 0x4f7c30, with this + 0xd4, and 0x4f8ad0): the five integer pairs at +0x64 destroyed, the last first, with the
// destructor that does nothing (CRT_VectorDtorIterator, Stub_Ret); nothing else.
void __fastcall Nav_Dtor(u8 *object);

// 0x4f7c30 (thiscall; an adjustor): Nav_Dtor on the block at this + 0xd4.
void __fastcall Nav_DtorAtD4(u8 *owner);

// 0x50f280 (&out, &point): point - pos in the nav's local frame (16.16 products, local_xaxis at +0x20, local_yaxis at
// +0x28): out = (d . xaxis, d . yaxis), y written first. Returns out.
s32 *__fastcall Nav_WorldToLocal(const u8 *nav, void *, s32 *out, const s32 *point);

// 0x50f870 (&out, time): out = pos + vel * time (IntPair_MulFixed, IntPair_Add, through temporaries). Returns out.
s32 *__fastcall Nav_PredictPos(const u8 *nav, void *, s32 *out, s32 time);

// 0x50fd40: how far along its path the nav is, 16.16: 1.0 when the path is finished (byte +0x60 is 1) or its total
// (path_total_dist, +0xa0) is 0, else 1.0 - (+0xa4 + +0xa8 + the octagonal distance to the path point +0x5c of the
// five at +0x64) << 16 / total, one IDIV (it faults, sic, when the quotient passes 31 bits).
s32 __fastcall Nav_PathProgress(const u8 *nav);
}
