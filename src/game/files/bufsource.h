// The buffered source (0x638ef0-0x639228, vtable 0x703f78): a RefObject over another source
// that reads through a buffer of a fixed capacity. The buffer holds a window [start, end) of
// the wrapped source; reads copy from the window, refill it when the position is outside it
// and the read is smaller than the buffer, and read the rest straight from the wrapped source.
// The wrapped source's own position is tracked (sourcePos) so it is only sought, relative, when
// it differs. Writes go straight through and trim or move the window.
//
// The originals' quirks are kept: the constructor leaves the window's end unset; the buffer is
// freed only by the teardown slot (+0x24), never by the destructor (vtable +0 is the tracked
// source's, 0x638860), and the pointer is kept; Read answers 0 without a wrapped source but
// Write, Attach and the helpers use it unchecked; a raw read whose seek misses answers 0 with
// the missed position kept, so a refill then empties the window there; Seek clamps a position
// past the end to the end and answers -1, keeps the position on an unknown origin, and compares
// unsigned (a negative position is past the end); a write over the window's position trims the window to end there,
// one from before it that ends inside it moves its start up, and one that crosses it entirely
// empties it (start = end).
#pragma once

#include "../../../re/types.h"
#include "handles.h"

struct BufferedSource {    // vtable 0x703f78
    RefObject base;        // base.link: the wrapped source
    u8 *buffer;            // +0xc
    u8 *window;            // +0x10: the byte at start (the buffer, after a refill)
    u32 position;          // +0x14
    u32 start;             // +0x18: the window, in the wrapped source
    u32 end;               // +0x1c
    u32 sourcePos;         // +0x20: the wrapped source's position
    u32 capacity;          // +0x24
};
static_assert(sizeof(BufferedSource) == 0x28, "BufferedSource is 0x28 bytes");

extern "C" {
BufferedSource *__fastcall BufferedSource_Ctor(BufferedSource *s);  // 0x638ef0
// 0x638f20 (&result, capacity): the buffer allocated; 0, or -1 when that failed.
s32 *__fastcall BufferedSource_Open(BufferedSource *s, void *edx, s32 *result, u32 capacity);
void __fastcall BufferedSource_Free(BufferedSource *s);  // 0x638f70 (+0x24): the buffer freed
// 0x638f80 (buffer, count; +4): 0 without a wrapped source.
u32 __fastcall BufferedSource_Read(BufferedSource *s, void *edx, void *buffer, u32 count);
// 0x639050 (buffer, count; +8): the wrapped write, 0 when the wrapped seek misses.
u32 __fastcall BufferedSource_Write(BufferedSource *s, void *edx, const void *buffer, u32 count);
// 0x639000 (offset, origin; +0xc): the new position, or -1.
s32 __fastcall BufferedSource_Seek(BufferedSource *s, void *edx, s32 offset, u32 origin);
s32 __fastcall BufferedSource_Tell(const BufferedSource *s);  // 0x6390f0 (+0x14): -1 unattached
// +0x10 (the wrapped size, -1 unattached) is StreamBase_Size (0x639100, handles.h): one function.
// 0x639110 (source; +0x28): RefObject_SetLink, both positions from the wrapped tell.
u32 __fastcall BufferedSource_Attach(BufferedSource *s, void *edx, RefObject *source);
// 0x639130 (buffer, count): the raw read, added to the position.
u32 __fastcall BufferedSource_ReadDirect(BufferedSource *s, void *edx, void *buffer, u32 count);
// 0x639150 (buffer, count): what the window holds at the position, up to count.
u32 __fastcall BufferedSource_CopyWindow(BufferedSource *s, void *edx, void *buffer, u32 count);
u32 __fastcall BufferedSource_Fill(BufferedSource *s);  // 0x6391b0: the window refilled
// 0x6391e0 (buffer, count): the wrapped source sought to the position, then read.
u32 __fastcall BufferedSource_ReadRaw(BufferedSource *s, void *edx, void *buffer, u32 count);
}

struct BufferedSourceEnv {
    void *(__cdecl *alloc)(u32 size);  // Mem_Alloc (0x67acc0)
    void(__cdecl *free)(void *block);  // Mem_Free (0x67acb0)
};
// The calls go through env (nullptr: the game's).
void BufferedSource_SetEnv(const BufferedSourceEnv *env);
