// The video library's macroblock decoder (16x16 blocks of four 8x8 luma squares and two chroma values, the EA TGQ kind: unconfirmed).
// Colours come from the tables built at run time: luma 0xfb7930, the two chroma ones 0xfb7d30 / 0xfb7b30 (128 dwords each, summed), each
// indexed by (value * [0xfb7820]) (a 16.16 product, rounded by its bit 15) >> 1 & 0x7f. Reimplementation of 0x671340 and 0x671650. The
// names are unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x671340 (cdecl (y0, y1, y2, y3, c0, c1, out, pitch: the six values signed bytes); name unconfirmed): a flat macroblock at 16 bits a
// pixel: each 8x8 square (y0 at out, y1 16 bytes right, y2 pitch * 8 down, y3 both) filled with luma[y] + chroma 0xfb7d30[c0] +
// 0xfb7b30[c1] (its low 16 bits), the rows (pitch >> 1) * 2 bytes apart.
void __cdecl TgqVideo_FillBlocks16(s32 y0, s32 y1, s32 y2, s32 y3, s32 c0, s32 c1, u8 *out, u32 pitch);

// 0x671650 (cdecl, as TgqVideo_FillBlocks16; name unconfirmed): the same at 32 bits a pixel: the squares 32 bytes right and pitch * 8
// down, the rows (pitch >> 2) * 4 bytes apart.
void __cdecl TgqVideo_FillBlocks32(s32 y0, s32 y1, s32 y2, s32 y3, s32 c0, s32 c1, u8 *out, u32 pitch);

// 0x66fd70 (fastcall (r, g; b, unused, red bits, green bits, blue bits, red shift, green shift, blue shift), pops 32; name unconfirmed):
// a colour packed twice into a pixel pair: each channel v rounded to its bits ((v + (1 << (7 - bits))) / (1 << (8 - bits)), signed;
// below 0 (v - that) / it), clamped to 1 - (1 << bits) .. (1 << bits) - 1, into the low half at its shift; and truncated (v / (1 << (8 -
// bits))) into the high half at shift + 16, after a borrow chain: a negative rounded blue takes one from the rounded green, that from
// the rounded red, that from the truncated blue, that from the truncated green and that from the truncated red. Each value is masked to
// its bits; with the bits not summing to 16 both halves get their top bit (0x80008000). Shift counts are taken mod 32, as the CPU does.
u32 __fastcall TgqVideo_PackColour(s32 r, s32 g, s32 b, u32 unused, u32 red_bits, u32 green_bits, u32 blue_bits, u32 red_shift,
                                   u32 green_shift, u32 blue_shift);

// 0x66fc10 (fastcall (unused, red bits; green bits, blue bits, red shift, green shift, blue shift), pops 20; name unconfirmed): the
// colour tables for a pixel format (TgqVideo_PackColour): for i 0-127 with c = (s8)(2i + 1), luma 0xfb7930[i] = (c + 0x80) as grey,
// 0xfb7d30[i] = (0, c * -0.3441, c * 1.7720) and 0xfb7b30[i] = (c * 1.4020, c * -0.7141, 0), the products 16.16 rounded by bit 15.
void __fastcall TgqVideo_BuildTables(u32 unused, u32 red_bits, u32 green_bits, u32 blue_bits, u32 red_shift, u32 green_shift,
                                     u32 blue_shift);

// 0x671000 (cdecl (quality); name unconfirmed): the dequantiser 0xfb7820 (64 dwords) from the base matrix 0x705c88: with d = 100 -
// quality, base = d * 22 / 100 + 8 and step = d * 50 / 100 + 10 - base (signed, truncated), entry (row, k) = the matrix's * ((row + k)
// * step / 14 + base) << 16, as 16.16 rounded by bit 15 (the products wrapping).
void __cdecl TgqVideo_SetQuality(s32 quality);

// 0x673574 (luma in ESI, out in EDI, chroma in EBX: hand-written assembly hooked through RegThunk_TgqVideo_PixelRow16; name
// unconfirmed): 16 pixels at 16 bits from 16 luma dwords and 8 of each chroma (chroma[k], chroma[k + 0x40]), each value's bits 17-23 the
// index: out word 2k = luma[2k] + 0xfb7d30[chroma[k]] + 0xfb7b30[chroma[k + 0x40]] (the low half), 2k + 1 with luma[2k + 1]. Pair by
// pair, each pair's four dwords read before its two words are written. It reads out's dwords 0 and 8 and drops them (the first before
// anything, the second before pair 4): kept, as reads that fault where the original's do.
void __cdecl TgqVideo_PixelRow16(const u32 *luma, u16 *out, const u32 *chroma);

// 0x6732e9 (luma in ESI, out in EDI, chroma in EBX: hand-written assembly hooked through RegThunk_TgqVideo_PixelRow32; name
// unconfirmed): as TgqVideo_PixelRow16 at 32 bits a pixel (out dwords 2k and 2k + 1, the sums whole), the same dropped reads of out's
// dwords 0 and 8.
void __cdecl TgqVideo_PixelRow32(const u32 *luma, u32 *out, const u32 *chroma);

// 0x672e60 (cdecl; hand-written assembly leaving the coefficient counter in EDI, which its thunk RegThunk_TgqVideo_DecodeCoefficients
// sets from the returned value: its callers 0x67380f / 0x673fee pass EDI on to 0x67673c as the out pointer; name unconfirmed): a block's
// 64 coefficients into 0xfb7600 from the bit reader (source 0xfb7920, bits 0xfb7928 least significant first, their count 0xfb7924:
// read in, written back). The DC: the buffer's low byte (signed) << 16 times the dequantiser 0xfb7820[0] (16.16 rounded by bit 15), then
// 8 bits dropped. Then for the counter 0x3f down while above 0, at the zigzag index 0xfbb5ac[counter], the jump table 0xfbbaac[bits & 7]
// picks: 0x672ede zero; 0x672ebc the dequantiser's entry (0x672ecc its negation); 0x672eeb two zeros (the counter down one more); 0x672f49 a
// run of (bits & 0xff) >> 3 pairs of zeros (the counter down two a pair, then up one; a run of 0 wraps, 2^32 pairs, sic), 0x672f36 a zero
// before the same; 0x672f7a ((bits & 0xfc) << 24 >> 10 (signed) times the entry, or with bits & 0xfc = 0xfc the next byte's (signed)
// << 16). Runs and values use 8 bits (a byte read in each time at count - 8), the rest 3 (a byte read in when fewer than 8 are left).
// The table in the exe has 0x672ede, 0x672f49, 0x672ebc, 0x672f7a, 0x672eeb and then three dwords that are not code: codes 5-7 jump there
// (called here, so the port faults as the original does, sic). Returns the counter (0 or below).
s32 __cdecl TgqVideo_DecodeCoefficients();

// 0x673fee (cdecl (data, out, pitch in bytes); name unconfirmed): a coded 16x16 macroblock at 16 bits a pixel. The bit reader set to
// data (its first byte in, 8 bits, the source past it), then six blocks, each TgqVideo_DecodeCoefficients, the 8 columns of 0xfb7600
// (0x20 bytes apart) through MovieIdct_Column and the 8 rows of 0xfb7700 (0x24 apart) through MovieIdct_Row: the luma squares into
// 0xfb7000, 0xfb7020, 0xfb7200 and 0xfb7220 (rows 0x40 bytes apart), the chroma into 0xfb7400 and 0xfb7500 (rows 0x20 apart); then
// 16 rows of TgqVideo_PixelRow16 (luma 0xfb7000 + 0x40 j, chroma 0xfb7400 + 0x20 (j / 2), out + pitch * j). As the original runs, the
// columns' out is EDI: 0xfb7700 + 4i goes on the stack, where 0x67673c never looks, and EDI is the counter TgqVideo_DecodeCoefficients
// left there (0 or below), so the first column write faults (sic) and nothing after it is reached.
void __cdecl TgqVideo_DecodeMacroblock16(const u8 *data, u16 *out, u32 pitch);

// 0x67380f (cdecl (data, out, pitch in bytes); name unconfirmed): as TgqVideo_DecodeMacroblock16 at 32 bits a pixel (the rows through
// TgqVideo_PixelRow32): the same fault (sic).
void __cdecl TgqVideo_DecodeMacroblock32(const u8 *data, u32 *out, u32 pitch);

// 0x671230 (fastcall (player, data; out, unused, pitch in bytes), pops 12; name unconfirmed): a frame's macroblocks at 16 bits a
// pixel, rows of them while y (from 0, by 16) is below the settings' +0x60, in each while x is below +0x64 (both unsigned, read again
// each pass; the settings are the player's dword +0): a type byte, then by type (signed) above 0xc TgqVideo_DecodeMacroblock16 (data
// past the byte), 3 TgqVideo_FillBlocks16 (b0 four times, b1, b2), 6 (b0-b5), 0xc (b0, b2, b4, b6, b8, b10), any other nothing; data
// then moves past the byte and type more, out 0x20 bytes right, and each row starts (pitch >> 1) << 5 bytes below the last.
void __fastcall TgqVideo_DecodeFrame16(u8 *player, const u8 *data, u8 *out, u32 unused, u32 pitch);

// 0x671540 (fastcall, as TgqVideo_DecodeFrame16; name unconfirmed): the same at 32 bits a pixel: TgqVideo_DecodeMacroblock32 and
// TgqVideo_FillBlocks32, out 0x40 bytes right, rows (pitch >> 2) << 6 bytes apart.
void __fastcall TgqVideo_DecodeFrame32(u8 *player, const u8 *data, u8 *out, u32 unused, u32 pitch);

// 0x670090 (fastcall (a, b; c, d), pops 8; name unconfirmed): four signed values packed into a dword's bytes (d the top, c the bottom)
// so that adding the dword adds each: a borrow taken from the byte above each negative one (c's from b, then b's from a, a's from d,
// each tested after its own borrow).
u32 __fastcall TgqVideo_PackLanes(s32 a, s32 b, s32 c, s32 d);
}
