// An animated object put down on the park grid: reimplementation of 0x467030.
//
// The animator (see animator.h): +8 the holder, whose +4 is the model and +0x10 its turn in
// degrees (0-359); the model's +0x78 points at its transform block, the matrix at +0x10.
#pragma once

#include "../../../re/types.h"

extern "C" {
// 0x467030 (animator, x, y, cellX, cellY, angle, height, flags, width, height in cells).
// Flag 2: the footprint, width by depth turned by angle (0 and 180 keep it, any other angle
// swaps them): the model's flag byte +0x31 |= 4 and +0xa8 = the bytes x, x + w - 1, y,
// y + d - 1 (high to low); the animator's +0xc0 = y | x << 16 and +0xc4 = (y + d - 1) |
// (x + w - 1) << 16 (low words).
// Flag 0x20: the holder's turn += angle, mod 360 (made positive); the matrix turned about y
// by angle degrees (the sine table at 0x7b095c) and its translation (cellX * grid +0x10 +
// grid +8, height, cellY * grid +0x14 + grid +0xc), the grid being 0x7a0854's +0x6c (read
// either way).
void __cdecl Animator_Place(u8 *animator, s32 x, s32 y, s32 cellX, s32 cellY, s32 angle, u32 height, u32 flags, s32 width, s32 depth);
}
