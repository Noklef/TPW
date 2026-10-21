// The video library's block copies into the macroblock buffers (0xfaa860 the 16x16 one, 0xfaa760 the 8x8 ones: rows of dwords whose
// byte 2 is written), each source byte plus a bias (wrapping). Reimplementation of 0x676a70 and 0x676b50. The names are unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x676a70 (fastcall (source, swapped; pitch in dwords, out, bias), pops 12; name unconfirmed): 8 rows of 8 into out's dwords' byte 2
// (rows 0x40 bytes apart), from the source rows pitch * 4 bytes apart: with swapped the bytes 2, 7, 6, 11, 10, 15, 14, 19 of the row,
// else 3, 2, 7, 6, 11, 10, 15, 14.
void __fastcall RefBlock_Copy16(const u8 *source, s32 swapped, s32 pitch, u8 *out, u8 bias);

// 0x676b50 (fastcall (base, offset; pitch, out, bias), pops 12; name unconfirmed): 8 rows of 8 from base + offset (every fourth byte,
// the rows pitch * 8 bytes apart) into out's dwords' byte 2 (rows 0x20 bytes apart).
void __fastcall RefBlock_Copy8(s32 base, s32 offset, s32 pitch, u8 *out, u8 bias);
}
