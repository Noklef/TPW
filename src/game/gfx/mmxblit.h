// The software renderer's picture copies to the screen surface (hand-written MMX assembly: MOVQ moves, EMMS at the end; called from
// 0x573d00's present). The MMX arithmetic is done on 64-bit integers lane by lane. Reimplementation of 0x678790, 0x678c20,
// 0x678d70 and 0x6787f0. The names are unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x678790 (cdecl (source, out, width, rows, source pitch, out pitch); name unconfirmed): rows of (width >> 2) * 16 bytes copied, 16
// bytes at a time (two qwords read, then written); each row's start moves by its pitch. Both counts are tested after their pass, so
// a count of 0 runs 2^32 times (sic).
void __cdecl MmxBlit_Copy16(const u8 *source, u8 *out, u32 width, u32 rows, s32 sourcePitch, s32 outPitch);

// 0x678c20 (cdecl (source, out, width, rows, source pitch, out pitch, depth, depth pitch, clear, fill: a qword, its high dword past
// the caller's ten arguments); name unconfirmed): width into 0xfab484, rows 0xfab4a0, the depth pitch 0xfab4b0. Rows (0xfab4a0, read
// once; tested after each, so 0 runs 2^32 times, sic) of 32-byte passes while the width (0xfab484, read again each row) less 8 a
// pass stays above 0 (signed): with clear's low byte 0 (fill's low dword then into 0xfac51c and 0xfac518) the source's four qwords
// copied to out; else read, the source's four set to fill, out's to them, and depth's two to the qword 0x77e580 (16 bytes a pass,
// its row moving by 0xfab4b0, read again each row).
void __cdecl MmxBlit_Copy32(u8 *source, u8 *out, u32 width, u32 rows, s32 sourcePitch, s32 outPitch, u8 *depth, u32 depthPitch,
                            u32 clear, u32 fillLow, u32 fillHigh);

// 0x678d70 (cdecl (source, out, width, rows, source pitch, out pitch); name unconfirmed): 32-bit pixels to 16 bits, dithered: for each
// row (counted up from 0 until it equals rows, tested after each: 0 runs 2^32 times, sic) the two qwords at 0x77e540 + (row & 3) * 16
// into 0xfab4e0 / 0xfab4e8; then width >> 2 passes (tested after each, the same) of 16 bytes in, 8 out: each qword PADDUSB its dither
// qword (read from 0xfab4e0 / 0xfab4e8 each pass), (q & [0xfbe198]) PMADDWD [0xfbe190] >> 6 (dwords) OR (q & [0xfbe1a0]) >> [0xfbe1a8]
// (dwords; the three masks read once), each dword's low word paired with the next's (q | q >> 16) and the two qwords' low dwords
// written. The next pass's 16 bytes are read before this pass's 8 are written, so each row reads 16 bytes past its end (sic).
void __cdecl MmxBlit_Dither16(const u8 *source, u8 *out, u32 width, u32 rows, s32 sourcePitch, s32 outPitch);

// 0x6787f0 (cdecl (source, out, width, rows, source pitch, out pitch, depth, depth pitch, clear, fill); name unconfirmed): 32-bit
// pixels to 16 bits, dithered, as MmxBlit_Dither16 but with the source, out and depth pointers moved to the row's end (by width * 4,
// * 2, * 2) and indexed by a count from -width (0xfab4d8, read again each row) up by 4 while below 0 (signed, tested after each pass);
// rows until the count from 0 equals 0xfab4fc (read again each row; 0 runs 2^32 times, sic); 0xfab4b4 the depth pitch. With
// [0xfbe058] = 5 (read once) the halves (q & [0xfbe1a0]) | (q & [0xfbe198]) PMADDWD [0xfbe190], >> 6, PACKSSDW, one pass behind its
// reads: the last pass's group written over the row's last 8 bytes, so with a width of 4 or less that is the group past the row's end
// (sic); else (q & [0xfbe198]) PMADDWD >> 6 | (q & [0xfbe1a0]) >> 5, paired as in MmxBlit_Dither16, each pass reading the next group.
// With clear's low byte set the fill (into 0xfac51c and 0xfac518, read back as the qword) is written over each source group read and
// the qword 0x77e580 into the depth's 8 bytes a pass, the dither then going through 0xfab4e0 / 0xfab4e8 as in MmxBlit_Dither16.
void __cdecl MmxBlit_Convert16(u8 *source, u8 *out, u32 width, u32 rows, s32 sourcePitch, s32 outPitch, u8 *depth, u32 depthPitch,
                               u32 clear, u32 fill);
}
