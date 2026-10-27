// The sound streams' block pools (the names are invented): freed blocks of a stream class's size are kept on a free list for the next
// new, under a critical section the pool allocates. The stream filter's (0x70a678: 0x18-byte blocks, the next block at +0x10; the lock at
// [0xfb206c], the list at [0xfb2068]), the looping stream's (0x70aba8: 0x24, +0x18; [0xfb2084], [0xfb2080]), the stream 0x70abf8's
// (0x14, +0x10; [0xfb2090], [0xfb208c]), the stream 0x70a6c8's (0x1c, +0x14; [0xfb20a8], [0xfb20a4]), the voice-coded samples' over
// memory (0x70ad60, core/voicesample: 0x40, +0x28; [0xfb20c0], [0xfb20bc]) and over a sub-stream (0x70adc0: 0x5c, +0x58; [0xfb20cc], [0xfb20c8]),
// the silence stream's (0x70ab58, core/silencestream: 0x30, +0x28; [0xfb2078], [0xfb2074]). Other sizes go straight to
// Mem_Free. As the MP2 decoders' pools (src/game/core/mp2pool). Reimplementation of each pool's Init, Shutdown, Purge, New
// and Delete (0x6c6340-0x6c6495, 0x6c68a0-0x6c69f5, 0x6c6bd0-0x6c6d25, 0x6c7300-0x6c7455, 0x6c79c0-0x6c7b15, 0x6c7e10-0x6c7f65,
// 0x6c65d0-0x6c6724; the silence pool's atexit registration 0x6c65c0 / 0x6c65f0 left out).
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x6c6340 / 0x6c68a0 / 0x6c6bd0 / 0x6c7300: the lock's 0x18 bytes from Mem_Alloc, initialised.
void __cdecl StreamFilterPool_Init();
void __cdecl LoopStreamPool_Init();
void __cdecl Stream70abf8Pool_Init();
void __cdecl Stream70a6c8Pool_Init();
void __cdecl VoiceSamplePool_Init();   // 0x6c79c0
void __cdecl VoiceStreamPool_Init();   // 0x6c7e10

// 0x6c6370 / 0x6c68d0 / 0x6c6c00 / 0x6c7330 (registered with atexit): purged, the lock deleted, freed and cleared.
void __cdecl StreamFilterPool_Shutdown();
void __cdecl LoopStreamPool_Shutdown();
void __cdecl Stream70abf8Pool_Shutdown();
void __cdecl Stream70a6c8Pool_Shutdown();
void __cdecl VoiceSamplePool_Shutdown();   // 0x6c79f0
void __cdecl VoiceStreamPool_Shutdown();   // 0x6c7e40

// 0x6c63a0 / 0x6c6900 / 0x6c6c30 / 0x6c7360: under the lock, every block on the free list freed (Mem_Free).
void __cdecl StreamFilterPool_Purge();
void __cdecl LoopStreamPool_Purge();
void __cdecl Stream70abf8Pool_Purge();
void __cdecl Stream70a6c8Pool_Purge();
void __cdecl VoiceSamplePool_Purge();   // 0x6c7a20
void __cdecl VoiceStreamPool_Purge();   // 0x6c7e70

// 0x6c63e0 / 0x6c6940 / 0x6c6c70 / 0x6c73a0 (cdecl size; the classes' operator new): 0 counts as 1; any size but the pool's: Mem_Alloc;
// the pool's: under the lock, the free list's first block, or a new one from Mem_Alloc when the list is empty.
void *__cdecl StreamFilterPool_New(u32 size);
void *__cdecl LoopStreamPool_New(u32 size);
void *__cdecl Stream70abf8Pool_New(u32 size);
void *__cdecl Stream70a6c8Pool_New(u32 size);
void *__cdecl VoiceSamplePool_New(u32 size);   // 0x6c7a60
void *__cdecl VoiceStreamPool_New(u32 size);   // 0x6c7eb0

// 0x6c6450 / 0x6c69b0 / 0x6c6ce0 / 0x6c7410 (cdecl block, size; the classes' operator delete): nothing for null; the pool's size: under
// the lock, onto the free list's front; any other: Mem_Free.
void __cdecl StreamFilterPool_Delete(void *block, u32 size);
void __cdecl LoopStreamPool_Delete(void *block, u32 size);
void __cdecl Stream70abf8Pool_Delete(void *block, u32 size);
void __cdecl Stream70a6c8Pool_Delete(void *block, u32 size);
void __cdecl VoiceSamplePool_Delete(void *block, u32 size);   // 0x6c7ad0
void __cdecl VoiceStreamPool_Delete(void *block, u32 size);   // 0x6c7f20
// The silence stream's: 0x6c65d0, 0x6c6600, 0x6c6630, 0x6c6670 and 0x6c66e0.
void __cdecl SilenceStreamPool_Init();
void __cdecl SilenceStreamPool_Shutdown();
void __cdecl SilenceStreamPool_Purge();
void *__cdecl SilenceStreamPool_New(u32 size);
void __cdecl SilenceStreamPool_Delete(void *block, u32 size);

}  // extern "C"
