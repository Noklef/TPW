// The video library's pixel row writers (hand-written assembly; they keep every register but EAX) and the YUV tables they use.
// Reimplementation of 0x676478, 0x6764bc, 0x670350, 0x6702d0, 0x6702f0 and 0x670210. The names are unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x676478 (cdecl (source, out, count); name unconfirmed): count bytes (a multiple of 4: the listing loops until it reaches 0) each
// written twice: every source dword b0 b1 b2 b3 as the dwords b0 b0 b1 b1 and b2 b2 b3 b3.
void __cdecl MoviePixels_Double(const u8 *source, u8 *out, u32 count);

// 0x6764bc (cdecl (source, out, count); name unconfirmed): count (at least 1: the listing tests after the first) source dwords
// (b0 b1 b2 b3: two pixels sharing b0 and b1, with the luma b3 and b2) to pixel pairs through the tables built at run time: the sums
// 0xfb9f30[b0] + 0xfb9730[b1] + 0xfb8f30[luma], each split into its bits 23-31, 11-19 and 0-8 to index 0xfba730, 0xfbabf8 and
// 0xfbb010, summed: the b3 pixel's low half and the b2 pixel's high half.
void __cdecl MoviePixels_FromYuv(const u32 *source, u32 *out, u32 count);

// 0x670350 (fastcall (format, player); x87; name unconfirmed): the YUV tables for a pixel format (+0 / +4 / +8 the red / green / blue
// bits, +0xc / +0x10 / +0x14 their shifts, +0x1c the depth) and the player's brightness (its settings' +0x78, in percent). For i
// -256..255 at (i - 0xc0) & 0x1ff, with c = i clamped to -64..63 and x = c + 0.5: 0xfb8f30 the luma sum ((c << 12) + c) << 11 + c +
// 0x4883d8a9; 0xfb9730 (0.5 - 0.3441x) & 0x1ff << 11 plus (0.5 + 1.772x) & 0x1ff; 0xfb9f30 ((0.5 - 0.7141x) & 0x1ff plus (0.5 + 1.402x)
// & 0x1ff << 12) << 11 (each truncated). Then the channel tables 0xfba730 (red, v = -0xb1..0x1b1), 0xfbabf8 (green, -0x85..0x185) and
// 0xfbb010 (blue, -0xe1..0x1e1), v odd: the level v * brightness / 100 (signed) clamped to 0-255, rounded to the channel's bits (capped
// at the top) at its shift, plus the truncated level at shift + 16 when the depth is 16 or less. The brightness, bits, shifts and depth
// are read again for each entry.
void __fastcall MoviePixels_BuildTables(const u32 *format, u8 *player);

// 0x6702d0 (fastcall (mask); name unconfirmed): a channel mask's bits: how many of bits 0-30 are set (bit 31 is not tested).
u32 __fastcall MoviePixels_MaskBits(u32 mask);

// 0x6702f0 (fastcall (mask); name unconfirmed): a channel mask's shift: its lowest set bit's place, the mask shifted right (arithmetic)
// until its low bit is set; a mask of 0 never ends (sic).
u32 __fastcall MoviePixels_MaskShift(s32 mask);

// 0x670210 (fastcall (unused, surface; out), pops 4; name unconfirmed): nothing for a null surface; else its DDPIXELFORMAT (0x20 bytes,
// cleared, its size set) read with GetPixelFormat (vtable +0x54; its answer not checked): at 8 bits a pixel out's +0x1c 8, else the
// red, green and blue masks' bits (MoviePixels_MaskBits) into out[0-2], their shifts (MoviePixels_MaskShift) into out[3-5], the masks
// or'd into out[6] and the bits' sum into out[7].
void __fastcall MoviePixels_ReadFormat(void *unused, void *surface, u32 *out);
}
