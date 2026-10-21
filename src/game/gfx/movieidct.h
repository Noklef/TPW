// The video library's 8-point inverse DCTs (hand-written assembly taking the block in ESI; their hooks are naked thunks,
// src/hook/regthunks.h, that pass the registers as these ports' ordinary cdecl arguments). Each does one 8-dword line: the
// first pass of a block writes it down a column of a 9-dword-wide buffer, the second along 8 consecutive dwords. The odd
// half goes through the x87 (fild, the float constants, then the 1.5 * 2^52 rounding constant added and the double's low
// dword taken: at the game's 24-bit precision that keeps only the top 3 bits of the rounded value, sic), the even half
// through IMUL by the Q31 constant's high dword doubled. Two copies of the constants: 0xfbc018 for the DCT codec's (src/game/gfx/dctvlc)
// (0x6780c1, 0x678224, scratch at 0xfbc008-0xfbc017 and the output pointer at 0xfbc004), 0xfbb598 for the TGQ video's (src/game/gfx/tgqvideo)
// (0x67673c, 0x67687b, 0x676965, scratch on the stack). Reimplementation of those five and the DCT codec's block driver 0x67832e. The names are unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x6780c1 (block in ESI; name unconfirmed): block's 8 dwords transformed down the column at [0xfbc004] (dwords 0x24 bytes
// apart), its intermediate values left at 0xfbc008-0xfbc017; when dwords 1-7 are all zero, dword 0 copied down the column.
void __cdecl DctIdct_Column(const s32 *block);

// 0x678224 (block in ESI; name unconfirmed): block's 8 dwords transformed into the 8 dwords at [0xfbc004] (no zero test), the
// intermediate values left at 0xfbc008-0xfbc017.
void __cdecl DctIdct_Row(const s32 *block);

// 0x67673c (block in ESI, out in EDI; name unconfirmed): as DctIdct_Column with the constants at 0xfbb598, down out's column.
void __cdecl MovieIdct_Column(const s32 *block, s32 *out);

// 0x67687b (block in ESI, out on the stack; name unconfirmed): as DctIdct_Row with the constants at 0xfbb598, into out. Its
// scratch is the 16 bytes past its argument, in the caller's frame (0x67380f's and 0x673fee's, which never read them back):
// left out.
void __cdecl MovieIdct_Row(const s32 *block, s32 *out);

// 0x676965 (block in ESI, out in EDI; name unconfirmed): as MovieIdct_Row, each result shifted right 17 (arithmetic).
void __cdecl MovieIdct_RowShift(const s32 *block, s32 *out);

// 0x67832e (cdecl (out, pitch in dwords); name unconfirmed): the DCT codec's 8x8 IDCT: [0xfbc000] = pitch * 4; the rows of the block
// 0xfbc14c (0x20 bytes apart) through DctIdct_Column down the columns of 0xfbc02c ([0xfbc004] = 0xfbc02c + 4i), then that buffer's rows
// (0x24 bytes apart) through DctIdct_Row into out's rows, [0xfbc004] = out and then [0xfbc004] + [0xfbc000] (both read again) for
// each. It keeps every register but EAX (left as the last row's value 6: no caller reads it).
void __cdecl DctIdct_Block(s32 *out, s32 pitch);
}
