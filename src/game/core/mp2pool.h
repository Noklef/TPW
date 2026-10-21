// The MP2 decoders' block pools (LbMpeg): freed decoder blocks of the two classes' sizes are kept on a free list for the next new, under
// a critical section the pool allocates (Mem_Alloc, 0x18 bytes). The buffer decoder's pool (0x2478-byte blocks; the next block at +0x2404):
// the lock at [0xfb2710], the list at [0xfb270c]. The stream decoder's (0x2c80-byte blocks; the next at +0x2478): [0xfb2718] and
// [0xfb2714]. Other sizes go straight to Mem_Alloc / Mem_Free. Reimplementation of 0x6c8f00-0x6c9030 and 0x6c9090-0x6c91c0 (the static
// initialisers 0x6c8f20 / 0x6c90b0 that register the shutdowns with the C runtime's atexit are left to the game); the names are invented.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x6c8f00 / 0x6c9090: the lock allocated (0x18 bytes) and kept, then InitializeCriticalSection on it (null as well).
void __cdecl Mp2BufferPool_Init();
void __cdecl Mp2StreamPool_Init();
// 0x6c8f30 / 0x6c90c0: the purge, DeleteCriticalSection and Mem_Free on the lock, and the lock 0.
void __cdecl Mp2BufferPool_Shutdown();
void __cdecl Mp2StreamPool_Shutdown();
// 0x6c8f60 / 0x6c90f0: under the lock, every block on the free list freed (Mem_Free), the list emptied.
void __cdecl Mp2BufferPool_Purge();
void __cdecl Mp2StreamPool_Purge();
// 0x6c8fb0 / 0x6c9140 (cdecl size; the classes' operator new): size 0 as 1; the pool's size: under the lock, the list's first block
// (taken off it) or a new one (Mem_Alloc; null when that fails); any other size: Mem_Alloc.
void *__cdecl Mp2BufferPool_New(u32 size);
void *__cdecl Mp2StreamPool_New(u32 size);
// 0x6c9030 / 0x6c91c0 (cdecl block, size; operator delete): nothing for null; the pool's size: under the lock, onto the free list's
// front; any other: Mem_Free.
void __cdecl Mp2BufferPool_Delete(void *block, u32 size);
void __cdecl Mp2StreamPool_Delete(void *block, u32 size);

}  // extern "C"
