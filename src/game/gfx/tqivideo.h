// The movie player's second DCT video (decoded through 0x671940 and the hand-written 0x674ab9-0x675767): its dequantiser, the MPEG-1
// intra matrix (0x705d88) scaled by the quality and by the IDCT's scales (0x705c88, as TgqVideo_SetQuality uses). TQI (EA's MPEG-like
// intra video) is a guess from the matrix and the (107 - quality) / 16 scale. Reimplementation of 0x6710c0 and 0x674ab9. The names are unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x6710c0 (cdecl (out, quality), x87; name unconfirmed): out's 64 dwords: at quality 100 [0x705c88 + i * 4] * 8 (16.16, rounded by
// bit 15 as the IMUL / SHL / SHR / ADC idiom); else the float (107 - quality) * 0.0625 (FILD, FSUBR, FMUL at the game's precision, then
// stored as a float) and each [0x705c88 + i * 4] * __ftol([0x705d88 + i * 4] * that * 65536) (16.16 the same, __ftol's low dword);
// then out[0] [0x705c88] * 8 in both.
void __cdecl TqiVideo_SetQuality(s32 *out, s32 quality);

// 0x674ab9 (hand-written; DC in EAX, dequantiser in ESI, the bit stream in EBP, its bit position in EDI, out in EDX, its pitch in bytes in ECX; the thunk, src/hook/regthunks.h; name unconfirmed): one block: the coefficients at their
// zigzag places (the 63 scan entries 0xfbb5b0, read from the end, as BL: an 8-bit index that wraps; the streams it wraps on overrun the block, not reproduced) each level * dequantiser, the DC dc * dequantiser[0]. The MPEG-1 first coefficient
// table in 17 bits by the first bits set (0xfbb69c, 0xfbb6cc, 0xfbb7ac twice, 0xfbb7ec, 0xfbb82c, 0xfbb86c, 0xfbb8ac): a dword of length << 16, run + 1 << 8, level, the sign the bit after the code; run + 1 0x41 the end, above it the escape (14 bits:
// run, level; level 0: the next 8 bits, 0x80: those less 0x100, above 0x80: less 0x100); under 0x20 the end, nothing consumed. No coefficient: the 8 x 8 block filled with the DC >> 17; else 8 MovieIdct_Column into 9-dword rows and 8
// MovieIdct_RowShift to out. Returns the bit position (EDI). Its EAX and EBX are not kept by the original; the callers set them again.
u32 __cdecl TqiVideo_DecodeBlock(s32 dc, const s32 *quant, const u32 *bits, u32 position, u8 *out, s32 pitch);
}
