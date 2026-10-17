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

}  // extern "C"
