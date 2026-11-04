// The heap with guard words around each block, the callers at 0x64c212-0x64c5d9. Reimplementation of 0x5d4ba0 and 0x5d4c20; the names are invented, unconfirmed.
// A block: the head tag (the dword at 0x774934), its size, the size's bytes, the tail tag (0x77492c); the count of blocks at 0xf827f4. A freed block's head becomes the tag at
// 0x7749c0. Messages go through the function pointer at 0x7748f4.
#pragma once

#include "../../../re/types.h"

// The CRT's calls (the ports by default; tests/fncmp/guardheap.py gives recorders).
struct GuardHeapEnv {
    void *(__cdecl *malloc)(u32 size);  // 0x67dcd0
    void(__cdecl *free)(void *block);   // 0x67ad90
};
void GuardHeap_SetEnv(const GuardHeapEnv *env);

extern "C" {

// 0x5d4ba0 (stdcall (size), pops 4): the block's user bytes (past the head and size). A size of 0 or below (signed): a message and 0. malloc answering 0 or -1: a message and 0.
void *__stdcall GuardHeap_Alloc(s32 size);

// 0x5d4c20 (stdcall (user), pops 4): 1 when the block is freed: both tags there and the size above 0 and below 0x6400000 (the tail read only then). Else 0 with a message: the
// pointer within 0x800 of null, the block freed already (its head the freed tag), or its guards bad.
u32 __stdcall GuardHeap_Free(void *user);
}
