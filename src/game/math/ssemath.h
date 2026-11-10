// The hand-written SSE matrix routines behind the adjustor thunks 0x579100-0x57913b (the game picks these or their 3DNow! twins by the CPU): reimplementation of 0x5f4eab, 0x5f4f81, 0x5f50c5, 0x5f5433.
// The first stack argument of each is the object the thunk adjusted, unused. Each operation is kept in the original's order (the sums are of floats). Names invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x5f4eab (source in EDX, dest in EDI, no frame): each of the 16 floats at source (any alignment) written four times to dest (16-aligned).
void __cdecl SseMath_ExpandMatrix(const float *source, float *dest);

// 0x5f4f81 (cdecl): the matrix expanded into the table 0x7753d0, then (count + 3) >> 2 batches (the sum wrapping, sic) of four vectors in 12 floats X, Y, Z from source + ((start & ~3) << 2) each transformed (no W, the bottom row added): the four
// results to dest (16-aligned), which moves on by stride << 2 a batch.
void __cdecl SseMath_TransformBatch(void *object, const float *matrix, u8 *dest, const u8 *source, u32 destStride, u32 start, u32 count);

// 0x5f50c5 (cdecl): count vectors (0 runs on until a fault, sic) by the matrix's rows. Mode 1: four floats in (source 16-aligned), four out. Mode 2: three in with the bottom row added, three out. Mode 3: three in, no bottom row, three out.
// Others: three in with the bottom row added, four out. The rows are read once, the vectors as they come.
void __cdecl SseMath_TransformVectors(void *object, const float *matrix, u8 *dest, const u8 *source, u32 destStride, u32 sourceStride, u32 count, u32 mode);

// 0x5f5433 (cdecl): out = a * b, 4 x 4 of rows. b is read first, a row by row as out's rows are written.
void __cdecl SseMath_Multiply(void *object, const float *a, const float *b, float *out);

// 0x5f525f (cdecl; the twin of Transform_ProjectToScreen, through the thunk 0x579120): count vertices (0 runs on until a fault, sic), each x, y, z, w, stride bytes
// apart, put on the screen by the render view's floats (0x8bcbc8: sx, sy +0x38, ox, oy +0x28, read before the loop, as the others). r is RCPPS of w (the CPU's
// approximation: the port runs the same instruction, so it gives what the original gives on the same machine); r' = 2 r - (w r) r its Newton step.
// Mode 1: x, y, z, w = (x sx) r' + (ox + 0), (y sy) r' + (oy + 0), 0 + +0x74, r'; returns +0x74 (read again) times the count (fild) in st0.
// Mode 2: x, y, z, w = (x sx') r + ox, (y sy') r + oy, z (0 + +0x58), r with sx' = sx + 0; returns the sum of the new z's, from 0, through the float 0x7754f0.
// Others: by z's bits against +0x60 (unsigned): not above, x, y, z, w = (x sx) q + ox, (y sy) q + oy, (z (+0x6c + t0)) q + +0x70, (w (0 + t1)) q2 + 0 with q = r,
// q2 = r r; above, the same with r' and +0x64, +0x68; t the 16-byte table 0x7754d0 (0, 1, 0, 0), read twice. Returns the z's sum as mode 2 does.
double __cdecl SseMath_ProjectToScreen(void *object, u8 *vertices, s32 stride, u32 count, u32 mode);
}

#include "../core/gamedata.h"

// Globals by address, by hand (.docs/09-globals.md; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(float[64], gSseMath_TransformBatch_7753d0, 0x007753d0, 0x100);
GAME_DATA(float, gSseMath_ProjectToScreen_7754f0, 0x007754f0, 0x4);

// Globals by address (tools/gamedata_codemod.py; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(u8[0x20], gSseMath_ProjectToScreen_7754d0, 0x007754d0, 0x20);
