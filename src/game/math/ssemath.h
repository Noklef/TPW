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
}
