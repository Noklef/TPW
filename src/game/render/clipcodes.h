// The clip codes of a run of points (hand-written assembly at 0x561bc0): reimplementation.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x561bc0 (out, codes, points, count, stride, mask; name unconfirmed; its own register use): for each of count points (count 0 runs 2^32 times,
// sic), stride bytes apart, of four dwords x, y, z, w (floats: doubling a pattern drops its sign, so the unsigned compares are of magnitudes), the
// 7-bit index sign(w), sign(z), |w| < |z|, sign(y), |w| < |y|, sign(x), |w| < |x| (first the top) picks a byte of the table at 0x879970; that byte
// and mask's low byte is the point's code, stored in turn from codes. The dword at out gets the AND of the codes (byte 0, starting at 0xff), their
// OR (byte 1) and the caller's ECX's high half (bytes 2-3: the local the function's PUSH ECX makes, sic), so it is taken here as a fastcall's
// first argument (the second, EDX, is shifted out). Returns out (EAX); pops 24 bytes.
u8 *__fastcall ClipCodes_Compute(u32 ecx, u32 edx, u8 *out, u8 *codes, const u8 *points, u32 count, u32 stride, u32 mask);

// 0x570d60 (stdcall-like, the caller's ECX passed on: the object at 0x879a20 in its callers; the same six arguments; name unconfirmed):
// ClipCodes_Compute with that ECX (and EDX the points, left from copying the arguments); returns out.
u8 *__fastcall ClipCodes_Run(u32 ecx, u32 edx, u8 *out, u8 *codes, const u8 *points, u32 count, u32 stride, u32 mask);

// 0x561840 (cdecl; name unconfirmed): when the block at 0x879a20 is not there yet, the 128 bytes of the table at 0x879970 are built: for each
// index i, x = +-1 or +-4 (bit 1 the sign, bit 0 the 4), y likewise from bits 3 / 2, z from bits 5 / 4 and w = 2 or -2 (bit 6), the byte is
// y > w (bit 5), y < -w (4), x > w (3), x < -w (2), z > w (1) and z < 0 (0); then the block is an 8-byte Mem_Alloc: 1, or on failure the
// error (3, 0xb0754000) pushed onto the engine's list and 0. 0 when the block was there.
u32 __cdecl ClipCodes_InitTable();
}
