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
