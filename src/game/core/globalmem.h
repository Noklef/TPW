// The game's GlobalAlloc / GlobalFree wrappers: reimplementation of 0x45b9d0-0x45ba14 and 0x45ba70. Both
// call through the game's import slots (GlobalAlloc 0x6fd1cc, GlobalFree 0x6fd26c), read at
// each call. (The file layer has its own pair, Mem_GlobalAlloc / Mem_GlobalFree 0x5f5660.)
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x45b9d0: GlobalAlloc(GMEM_ZEROINIT if bit 0 of zero, else GMEM_FIXED; size); null for a
// size of 0, without calling it.
void *__cdecl GlobalMem_Alloc(u32 size, u32 zero);

// 0x45ba00: GlobalFree(block)'s result (null when freed); null, without calling it, for null.
void *__cdecl GlobalMem_Free(void *block);

// 0x45ba70: a zeroed 0x40-byte block with count at +4 (and +0 cleared again); null, without
// allocating, for a count of 0. A failed allocation is written through anyway (sic).
void *__cdecl GlobalMem_AllocCounted(u32 count);
}
