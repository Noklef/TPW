// The hand-written 3DNow! matrix routines behind the adjustor thunks 0x579190, 0x5791a0 and 0x5791c0: reimplementation of 0x5f4730, 0x5f4844 and 0x5f4bd0, the twins of
// math/ssemath's. The game takes them only on a CPU with 3DNow! and without SSE: Transformer_Create (0x5781d0) picks the SSE kind when CPUID 1's EDX bit 25 is set and
// Transform_SseProbe runs MOVAPS, else this kind (2, vtable 0x7018f8) when CpuInfo's has3dNow (0x8bd53c, CPUID 0x80000001's EDX bit 31) is set, else the x87 one.
// No current Intel or recent AMD CPU runs 3DNow!, so these are plain float arithmetic (SSE scalars), each PFMUL and PFADD an IEEE single rounded to nearest, in the
// original's order of operations. 3DNow!'s own departures from IEEE (denormals taken and given as 0, no infinities or NaNs, overflow held at the largest normal) are
// not reproduced: for normal values whose results stay normal the two agree. The first stack argument of each is the object the thunk adjusted, unused. Each float is
// read before the store the original reads it before (the destination may lie on the matrix). Names invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x5f4730 (cdecl): (count + 1) >> 1 pairs (the sum wrapping; 0 runs on until a fault, sic) of vectors from batches of four in 12 floats X, Y, Z, the first
// at source + start * 4 + ((start & ~3) << 2), each transformed by the matrix's rows (no W, the bottom row added) to four floats at dest, which moves on by
// destStride a vector. The source steps 8 bytes a pair, and 0x28 more when its offset (start * 4, then kept below 0x20) passes a batch.
void __cdecl Amd3DNow_TransformBatch(void *object, const float *matrix, u8 *dest, const u8 *source, u32 destStride, u32 start, u32 count);

// 0x5f4844 (cdecl): count vectors (0 runs on until a fault, sic) by the matrix's rows. Mode 1: four floats in, four out. Mode 2: three in with the bottom
// row added, three out (the third sum adds it before z's product). Mode 3: three in, no bottom row, three out. Others: three in with the bottom row added,
// four out. Modes 2 and others read the bottom row once, mode 3 the third column's floats 2, 3, 10 and 11; the rest as each vector comes.
void __cdecl Amd3DNow_TransformVectors(void *object, const float *matrix, u8 *dest, const u8 *source, u32 destStride, u32 sourceStride, u32 count, u32 mode);

// 0x5f4bd0 (cdecl): out = a * b, 4 x 4 of rows, each out float (a0 b0 + a2 b2) + (a1 b1 + a3 b3); a pair of out's floats a time, the second stored first.
void __cdecl Amd3DNow_Multiply(void *object, const float *a, const float *b, float *out);
}
