// The strip animation of an object that grows or shrinks by a factor t (its +0xb8, stepped by
// +0xbc in 0x4547f0 / 0x4548f0): reimplementation of 0x4543c0 and 0x4547c0.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x4543c0: when the object's flags (+4) have 2, for each of the [+0xb0] strips i (from 0, 4
// vertices each from vertex 2 = 4 i + 2, of the shape at +0xb4: grouped vertices at +0x10, y
// 0x10 into each 0x30-byte group; texture coordinates at shape +0x24 -> +4 -> +0x78 -> +0x68,
// u and v 0x10 apart in 0x20-byte groups):
//   v = 1 - ((i / n * 0.8 - (t - 0.25)) * 4), then 0 when below 0 (or NaN) and 1 when above 1;
//   w = (0.8 for odd i, else 1) * v; the y of vertices j - 2 and j - 1 = that of j and j + 1
//   plus 10 w; the coordinates of j = (0, 1 - w) and of j + 1 = (1, 1 - w).
// Nothing for a count of 0 or less.
void __cdecl StripAnim_Apply(const u8 *object, float t);

// 0x454670 (cdecl (object, shown); name unconfirmed): when the object's flags (+4) have 2, its strip owner (+0xb4, an animator) is
// shown or hidden. Shown: the owner's matrix (+8 -> +0x28) becomes the identity (Matrix_Identity) with the translation of the
// holder's (+8) model's (+4) root (+0x78 -> +0x40..+0x48), turned about y by the holder's +0x10 degrees (fild; fmul 0.0174533 to a
// float; then as Matrix_RotateYAngle, inlined); the model's 0x18 bytes at +0x80 are copied to +0x80 of the owner's +0x24 -> +4; and
// the owner joins the grid over (+0xc0, +0xc4) (Animator_AddToGrid). Hidden: the dword at owner +8 -> +0x2c points to gets
// owner +0x24 -> +4 -> +0x50, the flags trade 0xe00 for 0x100, and the owner leaves the grid (Obj_RemoveRectAndClear14).
void __cdecl StripAnim_Show(u8 *object, u32 shown);

// The frame pointer below: the dword the owner's (+0xb4) +8 -> +0x2c points to = its +0x24 -> +4 -> +0x50 plus 0, 8, 0x10 or 0x18
// for frame 1, 2, 4 or 8 (flags 0x100, 0x200, 0x400, 0x800).
//
// 0x454550 (cdecl (object, frame); name unconfirmed): with strips (+0xb0), the object starts growing in frame: for frame 1 when its
// flags have 0x80 or no frame flag, for 2, 4 or 8 when that frame's flag is clear, the frame pointer is set and the frame flags become
// that frame's alone; other frames nothing. Then
// the step (+0xbc) = 0.2; StripAnim_Show(1) unless the flags have 0x20; flags 0x80 cleared, 0x60 set.
void __cdecl StripAnim_StartGrow(u8 *object, u32 frame);

// 0x4547f0 (cdecl (object); name unconfirmed): with strips, an object growing or shrinking (flags 0x20) is shown again
// (StripAnim_Show(1)) and its step set again (0.2 for flag 0x40, else -0.3 for 0x80); StripAnim_Apply at t (+0xb8); then the frame
// pointer from the first frame flag of 0x100, 0x200, 0x400, 0x800 (none: frame 1, and 0x100 set).
void __cdecl StripAnim_Refresh(u8 *object);

// 0x4548f0 (cdecl (object, dt); name unconfirmed): with a strip owner (+0xb4) and strips (+0xb0), an object growing or shrinking (flags
// 0xc0) steps t (+0xb8) by its step (+0xbc) times dt, held to 0..1 (the unrounded sum below 0 or a NaN: 0). A changed t is applied
// (StripAnim_Apply); an unchanged one (or a NaN) ends the change: shrinking (0x80) at 0 or less (or a NaN) the flags lose 0xa0 and
// the strips are hidden (StripAnim_Show(0)); growing at 1 or more the flags lose 0x40. (The flags as read before the step.)
void __cdecl StripAnim_Step(u8 *object, float dt);

// 0x4547c0: when it has strips (+0xb0), the step (+0xbc) = -0.3, and flags 0x20 trade 0x40 for 0x80.
void __cdecl StripAnim_StartShrink(u8 *object);

// 0x454190 (name unconfirmed): when the object's flags (+4) have 2 and there is a map grid (0x7a0854 +0x6c), the strips' ground
// heights: for each strip i (count [+0xb0]) and k = 0, 1, the end (+0xb0 -> +0xc)[2 i + k] packs (hi << 16 | lo); the cell
// Terrain_FootprintCell gives for the holder's turn (+8 -> +0x10), the footprint from (+0xc0 high word, low word) to (+0xc4 high, low)
// and the fractions hi / [+0xb0 +4], lo / [+0xb0 +8] (the cell left at (hi, lo) for an odd turn); y = (its grid height - the model's
// y (+8 -> +4 -> +0x78 -> +0x44)) * 0.5, and vertex 4 i + k of the shape at +0xb4 (y as StripAnim_Apply has it) gets y + 10, vertex
// 4 i + k + 2 gets y. The strip count and the model's y are read before the map.
void __cdecl StripAnim_FitGround(const u8 *object);
}
