// The shared buffer at 0x7aecd4 (12 bytes per entry; the motion code reads it as
// gTrackFlags, a u32 per node) and the frame transform that fills it with world positions:
// reimplementations of 0x462870, 0x4628a0 and 0x4628b0.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x462870: 0x7aecd4 = a zeroed block of 12 bytes for each of 0x400 entries (large) or
// [0x7a42ac] (not).
void __cdecl WorldVerts_Alloc(u32 large);

// 0x4628a0: frees the block at 0x7aecd4 (the pointer is left as it was).
void __cdecl WorldVerts_Free();

// 0x4628b0: the frame's vertices (+0x58 of them, u16; groups of 4 at +0x60, x, y and z 0x10
// apart, 0x30 per group) in world space into the block (x, y, z each): the frame's
// matrix (+0x10) times each parent's (+4 up the chain, Matrix_Mul), applied as rows:
// x' = (m8 z + m4 y) + m0 x + m12, and likewise y' (m9, m5, m1, m13) and z' (m10, m6, m2,
// m14), in that order on the x87.
void __cdecl WorldVerts_TransformFrame(const u8 *frame);
}
