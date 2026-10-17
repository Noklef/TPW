// The game's operator new / delete (Mem_Alloc 0x67acc0, Mem_Free 0x67acb0) for the ported code that allocates or frees blocks the rest of
// the game also does: GameHeap_Alloc / GameHeap_Free call the game's own functions (which are not replaced: they are the front of the
// C runtime's heap, so every allocation and free keeps going through one allocator). The tests give the code a stand-in through
// GameHeap_SetEnv, as the other units' environments do.
#pragma once

#include "../../../re/types.h"

struct GameHeapEnv {
    void *(__cdecl *alloc)(u32 size);  // Mem_Alloc (0x67acc0)
    void(__cdecl *free)(void *block);  // Mem_Free (0x67acb0)
};

// Allocation and release go through env (nullptr: the game's).
void GameHeap_SetEnv(const GameHeapEnv *env);

void *GameHeap_Alloc(u32 size);
void GameHeap_Free(void *block);

extern "C" {

u32 __cdecl Mem_AllocLogged(u32 size);

// 0x51b4a0 / 0x51b4d0 (cdecl; size / block): 0 when the argument is 0; otherwise the dword at 0x802bd8 is raised / lowered and the global memory of the file layer is
// allocated (Mem_GlobalAlloc) / freed (Mem_GlobalFree); returns what that returns.
u32 __cdecl Sound_Alloc(u32 size);

u32 __cdecl Sound_Free(void *block);

// 0x51f630 / 0x556ab0 / 0x51f660 (cdecl; size / size / block): Mem_GlobalAlloc / Mem_GlobalAlloc / Mem_GlobalFree when the argument is not 0 (the debug output of the
// others is gone), otherwise 0.
u32 __cdecl Mem_AllocChecked(u32 size);

u32 __cdecl Mem_FreeChecked(void *block);

// 0x579b40 (cdecl; size; name unconfirmed): 0 when the table of tracked global blocks (the count at 0x8bb214, pairs of dwords from 0x87b1f8)
// holds 0x8000; otherwise GlobalAlloc(GMEM_ZEROINIT, size) through its import slot, and a block got goes into both dwords of the next pair, the
// count raised (re-read). Returns the block (0 when that failed).
u32 __cdecl TrackedGlobal_Alloc(u32 size);

// 0x579b80 (cdecl; block; name unconfirmed): the pairs are searched from the last (count - 1, signed) down for one whose second dword is the
// block, then (none: the index -1) for one whose second dword is block + 4; none again: 0. Else the pair's first dword is freed (GlobalFree),
// the last pair (by the count re-read) moved into its place when it comes after it, and the count lowered. Returns 1.
u32 __cdecl TrackedGlobal_Free(u32 block);

// 0x58a4f0 (thiscall; count, pops 4; name unconfirmed): count dwords from TrackedGlobal_Alloc into +0x14; got, +8 = count and the block is
// returned, else 0.
u32 __fastcall TrackedArray_Alloc(u8 *array, void *, u32 count);

// 0x58a520 (thiscall; name unconfirmed): the block at +0x14 (when set) freed (TrackedGlobal_Free) and cleared, +8 = 0. Returns 1.
u32 __fastcall TrackedArray_Free(u8 *array);

// 0x586e20 / 0x587110 (thiscall; names unconfirmed): the tracked block at +0 (when set) freed (TrackedGlobal_Free) and cleared; the second
// returns 1 (the first is a destructor of the unwinding code's).
void __fastcall TrackedBlock_Dtor(u8 *holder);
u32 __fastcall TrackedBlock_Free(u8 *holder);

}  // extern "C"
