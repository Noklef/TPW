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

// 0x676290 (hand-written; luma in ESI, chroma in EDI, out in EBP; the thunk; name unconfirmed): a row of 16 pixels as 8 YUY2 dwords: from the table of dwords 8 bytes apart 0xfb8730 by
// luma[2k], chroma[k], luma[2k + 1], chroma[k + 0x40]: first | second << 8 | third << 16 | fourth << 24 (the whole dwords, overlapping bits or'ed).
void __cdecl TqiVideo_PixelRowYuy2(const u32 *luma, const u32 *chroma, u32 *out);

// 0x675d90 (hand-written; the same registers; name unconfirmed): a row of 16 pixels at 16 bits. For each k the chroma's sum 0xfb9b30[chroma[k]] + 0xfba330[chroma[k + 0x40]]; each of the
// two pixels' sum 0xfb9330[luma] + that, split: its bits 23-31, 11-19 and 0-8 index 0xfba730, 0xfbabf8 and 0xfbb010, or'ed. out[k]: the first's low half, the second's high half.
void __cdecl TqiVideo_PixelRowRgb16(const u32 *luma, const u32 *chroma, u32 *out);

// 0x6758c0 (hand-written; the same registers, ESP on the luma with its own kept at [0xfb792c], ESI not kept; name unconfirmed): as TqiVideo_PixelRowRgb16 with 16 out dwords, the two pixels' whole.
void __cdecl TqiVideo_PixelRowRgb32(const u32 *luma, const u32 *chroma, u32 *out);

// 0x674faf (cdecl (out, pitch in bytes, state); hand-written, all registers but EAX kept; name unconfirmed): a macroblock. The state: 64 dwords the dequantiser, +0x100 / +0x104 / +0x108 the three DC
// predictors, +0x200 the bit stream, +0x204 its bit position (written back). Six blocks (0x6747d0, also the other three's; it writes above its return address, and is not hooked): a DC difference
// (four luma: the top 7 bits by the bytes 0xfbb92c; two chroma: the top 8 by 0xfbb9ac; the byte's low nibble the code's length, its high nibble n the bits that follow, whose top bit clear means
// that value less (1 << n) - 1) added to the predictor, then TqiVideo_DecodeBlock: the luma squares +0, +0x20, +0x200, +0x220 of the stack's 0x800 bytes (pitch 0x40), the chroma +0x400 and +0x500 (0x20).
// Then 16 rows of TqiVideo_PixelRowYuy2 (luma + 0x40 j, chroma 0x400 + 0x20 (j / 2), out + pitch * j).
void __cdecl TqiVideo_DecodeMacroblockYuy2(u32 *out, s32 pitch, u8 *state);

// 0x67519d: the same through TqiVideo_PixelRowRgb16. 0x67538b: through TqiVideo_PixelRowRgb32. 0x675579: through TgqVideo_PixelRow16. (0x675767, the fourth of these, has no stack room: 0x6747d0 writes the
// blocks over its saved registers and return address, and it reads its rows from 0xfb7000. Not ported.)
void __cdecl TqiVideo_DecodeMacroblockRgb16(u32 *out, s32 pitch, u8 *state);
void __cdecl TqiVideo_DecodeMacroblockRgb32(u32 *out, s32 pitch, u8 *state);
void __cdecl TqiVideo_DecodeMacroblockTable16(u16 *out, s32 pitch, u8 *state);
}
