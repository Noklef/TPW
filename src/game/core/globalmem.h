// The game's GlobalAlloc / GlobalFree wrappers: reimplementation of 0x45b9d0-0x45ba50, 0x45ba70 and 0x45baa0. All
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

// 0x45baa0: GlobalMem_Free of the block at owner +0xb0 (the pointer is left as it was).
void *__cdecl GlobalMem_FreeAtB0(const u8 *owner);

// 0x45ba20: 0x7a1c60 = 1, which nothing reads.
void __cdecl GlobalMem_InitUnusedFlag();

// 0x45ba30, 0x45ba50: the unit's unused statics, 0x7a1c64 = 1.0f and 0x7a1c68 = 1 / 2^23.
void __cdecl GlobalMem_InitUnusedOne();
void __cdecl GlobalMem_InitUnusedEpsilon();

// 0x427bb0 (cdecl; name unconfirmed): GlobalMem_Free of the blocks at 0x74c8d4, 0x74c8c0 and 0x74c8b8 in turn, each pointer then cleared.
void __cdecl GlobalMem_FreeThree_427bb0();
}
