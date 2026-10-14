// The in-memory stream source (vtable 0x703868): reimplementation of 0x618df0-0x619240.
//
// A RefObject whose bytes live in a list of chunks of a fixed size (allocated whole, a
// 0x14-byte header then the data), positions counted from the first chunk: chunk n covers
// [n * size, (n + 1) * size). Reads stop at the size written; writes grow the list. The
// semaphore at +0x28 is for its users (vtable +0x2c hands it out); nothing here takes it.
//
// The originals' quirks are kept: a read that reaches the size moves past the last chunk,
// so a write after it starts a new chunk at the old chunk's end (leaving a gap); a seek
// that lands on a chunk's end leaves no chunk current, likewise. The destructor frees no
// chunks: the release before it (vtable +0x24, Clear) does.
#pragma once

#include "../../../re/types.h"
#include "../core/semaphore.h"
#include "handles.h"

struct MemoryChunk {
    MemoryChunk *prev;
    MemoryChunk *next;
    u8 *data;  // just past this header
    u32 start;
    u32 end;   // start + the chunk size
};
struct MemorySource {
    RefObject base;
    MemoryChunk *head;
    MemoryChunk *tail;
    MemoryChunk *chunk;  // where the position is, or nullptr
    u8 *cursor;          // the position in chunk
    u32 position;
    u32 size;
    u32 chunkSize;
    Semaphore lock;
};
static_assert(sizeof(MemorySource) == 0x2c, "MemorySource is 0x2c bytes");

extern "C" {
// __thiscall. 0x618df0 (chunk size) / 0x618e80 / 0x618e60 (flags; vtable +0).
MemorySource *__fastcall MemorySource_Ctor(MemorySource *m, void *edx, u32 chunkSize);
void __fastcall MemorySource_Dtor(MemorySource *m);
MemorySource *__fastcall MemorySource_DeletingDtor(MemorySource *m, void *edx, u32 flags);
// 0x618ee0 (buffer, count; vtable +4) / 0x618fc0 (+8): the bytes read / written.
s32 __fastcall MemorySource_Read(MemorySource *m, void *edx, void *buffer, s32 count);
s32 __fastcall MemorySource_Write(MemorySource *m, void *edx, const void *buffer, u32 count);
// 0x6190d0 (offset, origin 0-2; vtable +0xc): the new position, kept within [0, size];
// any other origin: the position, unchanged.
s32 __fastcall MemorySource_Seek(MemorySource *m, void *edx, s32 offset, u32 origin);
u32 __fastcall MemorySource_Size(MemorySource *m);  // 0x619150 (vtable +0x10)
u32 __fastcall MemorySource_Tell(MemorySource *m);  // 0x619160 (vtable +0x14)
// 0x619170 (&result): a new chunk at the end; -1 when the allocation fails.
s32 *__fastcall MemorySource_AddChunk(MemorySource *m, void *edx, s32 *result);
// 0x619200 (and 0x618ed0, vtable +0x24): frees the chunks, all back to 0.
void __fastcall MemorySource_Clear(MemorySource *m);
void __fastcall MemorySource_Teardown(MemorySource *m);
// 0x619240 (vtable +0x2c): the semaphore.
Semaphore *__fastcall MemorySource_GetLock(MemorySource *m);
}
