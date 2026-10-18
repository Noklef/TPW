// The clipper's new vertex where an edge crosses a clip plane (hand-written assembly at 0x561c40-0x562a6c, reached through a table of
// pointers): reimplementation. A vertex is x, y, z, w (floats, +0..+0xc), a colour (+0x10) and a specular (+0x14) dword and from +0x18
// pairs of texture coordinates up to the stride.
#pragma once

#include "../../../re/types.h"

extern "C" {

// Each (cdecl; out, a, b, stride; names unconfirmed): with the distances of a and b from the plane (dA, dB, kept as doubles at 0x8799f8
// and 0x879a00), the fraction t = 0.9999 dB / (dB - dA) (a double at 0x879a18) goes from b towards a: out's w and each texture
// coordinate (the count of pairs left and done at 0x879a0c / 0x879a08, a stride not 0x18 + a multiple of 8 running on, sic) are
// (a - b) * t + b. The colour and the specular are b's when a's is the same, else lerped by n = fistp(t * 255) (made a float in the
// argument a's slot, sic; through 0xfb34a4, kept at 0x879968): the bytes 0 and 2 as ((a & 0xff00ff) - (b & 0xff00ff)) * n >> 8 (signed)
// plus b's, masked (kept at 0x87996c), byte 1 as (a1 - b1) * n + (b1 << 8) and byte 3 that or a's byte 3 (sic; the parts kept at 0x879960
// and 0x879964). Of x, y and z, the plane's own is set from w (as the plane has it) and the others are lerped.

// 0x561c40: the plane y = w (the distance w - y): y = w.
void __cdecl ClipEdge_YW(u8 *out, const u8 *a, const u8 *b, s32 stride);
// 0x561ea0: y = -w (the distance y + w): y = -w.
void __cdecl ClipEdge_YNegW(u8 *out, const u8 *a, const u8 *b, s32 stride);
// 0x562100: x = w (w - x): x = w.
void __cdecl ClipEdge_XW(u8 *out, const u8 *a, const u8 *b, s32 stride);
// 0x562360: x = -w (x + w): x = -w.
void __cdecl ClipEdge_XNegW(u8 *out, const u8 *a, const u8 *b, s32 stride);
// 0x5625c0: z = w (w - z): z = w.
void __cdecl ClipEdge_ZW(u8 *out, const u8 *a, const u8 *b, s32 stride);
// 0x562820: z = 0 (the distance z): z = 0.
void __cdecl ClipEdge_Z0(u8 *out, const u8 *a, const u8 *b, s32 stride);
}
