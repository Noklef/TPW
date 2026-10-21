// The software renderer's picture copies to the screen surface (hand-written MMX assembly: MOVQ moves, EMMS at the end; called from
// 0x573d00's present). Reimplementation of 0x678790 and 0x678c20. The names are unconfirmed.
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
}
