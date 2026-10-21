// The sound streams' block pools (the names are invented): freed blocks of a stream class's size are kept on a free list for the next
// new, under a critical section the pool allocates. The stream filter's (0x70a678: 0x18-byte blocks, the next block at +0x10; the lock at
// [0xfb206c], the list at [0xfb2068]), the looping stream's (0x70aba8: 0x24, +0x18; [0xfb2084], [0xfb2080]), the stream 0x70abf8's
// (0x14, +0x10; [0xfb2090], [0xfb208c]) and the stream 0x70a6c8's (0x1c, +0x14; [0xfb20a8], [0xfb20a4]). Other sizes go straight to
// Mem_Free. As the MP2 decoders' pools (src/game/core/mp2pool). Reimplementation of 0x6c6450, 0x6c69b0, 0x6c6ce0 and 0x6c7410.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x6c6450 / 0x6c69b0 / 0x6c6ce0 / 0x6c7410 (cdecl block, size; the classes' operator delete): nothing for null; the pool's size: under
// the lock, onto the free list's front; any other: Mem_Free.
void __cdecl StreamFilterPool_Delete(void *block, u32 size);
void __cdecl LoopStreamPool_Delete(void *block, u32 size);
void __cdecl Stream70abf8Pool_Delete(void *block, u32 size);
void __cdecl Stream70a6c8Pool_Delete(void *block, u32 size);

}  // extern "C"
