// The node vertex scratch buffer: reimplementation of 0x450e60 and 0x450e80. 12 bytes (a
// vector) for each vertex of the biggest node loaded: the model loader raises the count at
// 0x79f7d0, the buffer is at 0x79f7e0.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x450e60: 0x79f7e0 = GlobalMem_Alloc(12 * count (0x79f7d0), zeroed).
void __cdecl Model_AllocVertexScratch();

// 0x450e80: frees 0x79f7e0 and clears it.
void __cdecl Model_FreeVertexScratch();
}
