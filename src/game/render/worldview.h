// The world view object [0x879d54] (0x2d8 bytes, made by CreateGlobal879d54_56b6c0; its near and far planes and field of view are set
// through 0x578be0 / 0x56b790): the extent of the ground it covers, which the procedural terrain texture sets from its header (0x56e6c0).
// Reimplementation of 0x56b830-0x56b93d (thiscall; x87 at the game's 24-bit precision). The names are unconfirmed.
//   +0x3c / +0x40 the x range's start and end, +0x4c its step, +0x54 its count of steps and +0x5c that less one;
//   +0x44 / +0x48 the y range's start and end, +0x50 its step, +0x58 its count and +0x60 that less one;
//   +0x38 the diagonal of the two ranges.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x56b850: +0x38 = the square root of dx * dx + dy * dy, dx = +0x40 - +0x3c and dy = +0x48 - +0x44 (dx's second factor as stored to a
// float first).
void __fastcall WorldView_UpdateDiagonal(u8 *view);

// 0x56b880 (start, end, step; pops 0xc): +0x3c = start, +0x40 = end, +0x4c = step; unless the step is 0 (or a NaN), +0x54 = __ftol((end -
// start) / step) (low half) and +0x5c = that - 1.
void __fastcall WorldView_SetRangeX(u8 *view, void *, float start, float end, float step);

// 0x56b8d0 (start, end, step; pops 0xc): the same for y: +0x44, +0x48, +0x50, then +0x58 and +0x60, computed twice over (sic).
void __fastcall WorldView_SetRangeY(u8 *view, void *, float start, float end, float step);

// 0x56b830 (thiscall; a, b, pops 8): +0x30 = a and +0x34 = b; returns a.
u32 __fastcall Obj_Set30_34(u8 *object, void *, u32 a, u32 b);
}  // extern "C"
