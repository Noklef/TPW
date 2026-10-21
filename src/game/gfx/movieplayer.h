// The movie player's frame headers and chunk helpers (the video library at 0x66e000-0x678000; names unconfirmed). The player's dword +0 is its settings
// (+0x60 height, +0x64 width (unconfirmed which), +0x78 the brightness in percent, +0x88 a flag that skips the header, +0xb4 the palette:
// three bytes an entry), +0x1e8 the current chunk's data, +0x1f4 its kind, +0x3a8 the kind read, +0x40 / +0x44 / +0x48 the kind 4 / 3
// header's flags and depth, +0x628 / +0x62c the palette header's first entry and count. Reimplementation of 0x670860, 0x670930,
// 0x670ab0, 0x677930, 0x677640, 0x6776c0, 0x6711a0, 0x6779e0, 0x67862f, 0x6786d8 and 0x670900.
#pragma once

#include "../../../re/types.h"

#include "moviepixels.h"

extern "C" {

// 0x670860 (thiscall; name unconfirmed): with the settings' +0x88 clear, the kind +0x1f4 copied to +0x3a8 and, with a chunk, its
// header read by kind: 1 MoviePlayer_ReadPalette; 3 the size as two signed words (+0x64, +0x60) and byte 4 into +0x48; 4 the same
// plus byte 7's bit 1 (as 2) into +0x40 and bit 0 into +0x44; 5-7 MoviePlayer_ReadSizeAt8; 2 and any other kind nothing.
void __fastcall MoviePlayer_ReadHeader(u8 *player, void *edx);

// 0x670930 (fastcall (player, data); name unconfirmed): the size as two unsigned words (+0x64 then +0x60), the words 4 and 6 into
// the player's +0x628 / +0x62c, then that many palette entries from data + 0xc, each byte * the brightness (32 bits, wrapping) / 100
// (unsigned) capped at 0xff, into the settings' +0xb4 on (the count read again each pass).
void __fastcall MoviePlayer_ReadPalette(u8 *player, const u8 *data);

// 0x670ab0 (fastcall (player, data); name unconfirmed): the size from the signed words at data + 8 (+0x64) and + 0xa (+0x60).
void __fastcall MoviePlayer_ReadSizeAt8(u8 *player, const u8 *data);

// 0x677930 (thiscall; name unconfirmed): the settings' 256 palette entries (+0xb4, three bytes) as pixels into +0x3b4: each channel
// >> (8 - its bits: +0x2c / +0x30 / +0x34, read once) << its shift (+0x38 / +0x3c / +0x40), or'ed; doubled into the high half with
// the flags +4's bit 6. The settings and shifts are read again for each entry.
void __fastcall MoviePlayer_BuildPixels(u8 *player, void *edx);

// 0x677640 (fastcall (out, data; count, bits), pops 8; name unconfirmed): count fields of bits each, little-endian from data's bit 0,
// sign-extended (by the field's top bit) into out's dwords, read from the last (as a dword at its byte, shifted by the bit in it).
void __fastcall MoviePlayer_UnpackFields(s32 *out, const u8 *data, u32 count, u32 bits);

// 0x6776c0 (fastcall (records, out; count), pops 4; name unconfirmed): for count above 0 records of 8 bytes (four colours, then 16
// two-bit indices in a dword), 16 bytes each into out: out[15 - k] the colour of index k (the low bits first).
void __fastcall MoviePlayer_ExpandBlocks(const u8 *records, u8 *out, s32 count);

// 0x6711a0 (thiscall on the view: +0 the settings, +4 / +8 the surface's width and height, +0xc / +0x10 the picture's, +0x14 / +0x18
// its place; name unconfirmed): the picture's size from the settings' +0x64 / +0x60, doubled across by the flags' bit 6 and down by bits
// 4-5; centred by bits 2 / 3 ((surface - picture) / 2, arithmetic), moved by +0x68 / +0x6c with bits 0 / 1; the width +0x70 with bit 7
// and the height +0x74 with bit 8. The flags are read again for each test.
void __fastcall MoviePlayer_PlaceView(u8 *view, void *edx);

// 0x6779e0 (fastcall (player, out; unused, pitch in bytes), pops 8; name unconfirmed): the frame's bytes (the player's +0x648) to out.
// At depth 8 (the settings' +0x48): with the flags' bit 6 MoviePixels_Double of height * width bytes (+0x60 * +0x64), else row by row
// width bytes copied (dwords then bytes, forward), the source moving by width and out by pitch & ~3. At any other depth each byte through
// the built pixels (the settings' +0x3b4, dwords): with bit 6 as a dword, else its low word; eight a pass while x (by 8) is below
// width, so a width short of a multiple of 8 runs past it, then out moves on by (pitch - width * 4) / 4 dwords (2 / 2 words: unsigned).
// The settings are read again at each test, the width after each row's copy.
void __fastcall MoviePlayer_DrawFrame(u8 *player, u8 *out, u32 unused, u32 pitch);

// 0x67862f (cdecl (out, source, count, bits); hand-written assembly; name unconfirmed): count unsigned bit fields of bits each (the mask
// (1 << bits) - 1, the shift mod 32) into out's dwords, four a pass while count less 4 a pass stays at 0 or above (signed), then one a
// pass for the rest: a field the dword at its byte rotated right by its bit in it (ROR, not a shift); in the passes of four the second
// and fourth fields are the first and third's dwords rotated on by bits, so a pair reaching past the dword's 32 bits takes the wrapped
// low bits (sic).
// 0x670900 (fastcall (player, data); name unconfirmed; MoviePlayer_ReadHeader's kind 4, jumped to): the size from the signed words at
// data and data + 2 into the settings' +0x64 / +0x60 (the settings read again for each), the depth byte 4 into +0x48, and byte 7's bits
// 1 and 0 into +0x40 / +0x44 (the byte read again for each).
void __fastcall MoviePlayer_ReadSizeFlags(u8 *player, const u8 *data);

void __cdecl MoviePlayer_UnpackIndices(u32 *out, const u8 *source, s32 count, u32 bits);

// 0x6786d8 (cdecl (indices, previous, out, split, offsets, codebook, pitch, across, rows); hand-written assembly; name unconfirmed): rows
// of across 4x4 blocks of bytes (pitch the row's bytes; out moves 4 a block and pitch * 3 at each row's end), a dword index each from
// indices: below split (signed) the block copied from previous at out's place plus offsets[index]; else the codebook's 16 bytes at
// (index - split) * 16, a dword a row. Both counts are tested after their pass (signed: 0 or below runs once).
void __cdecl MoviePlayer_DrawBlocks(const s32 *indices, u8 *previous, u8 *out, s32 split, const u32 *offsets, u8 *codebook, u32 pitch,
                                    s32 across, s32 rows);
}
