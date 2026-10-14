// A surface that owns its DirectDraw surface (0x6306c0-0x6307bc, thiscall; vtable 0x703998): a
// Surface with a flag for whether one has been made. The software cursor keeps three.
//
// The originals' quirks are kept: Release leaves a borrowed surface (borrowed set) alone, and
// then leaves created set too.
#pragma once

#include <cstddef>

#include "../../../re/types.h"
#include "surface.h"

struct SurfaceBuffer {
    u32 vtable;       // 0x703998: the deleting destructor only
    Surface surface;  // +4
    s32 created;      // +0x54
    s32 borrowed;     // +0x58: never set here
};
static_assert(offsetof(SurfaceBuffer, created) == 0x54, "SurfaceBuffer::created is at +0x54");
static_assert(sizeof(SurfaceBuffer) == 0x5c, "SurfaceBuffer is 0x5c bytes");

struct SurfaceBufferEnv {
    u32 vtable;                  // 0x703998
    void(__cdecl *free)(void *);  // 0x67acb0
};
// nullptr restores the game's.
void SurfaceBuffer_SetEnv(const SurfaceBufferEnv *e);

extern "C" {
// 0x6306c0: no colour key, no surface, an empty info; not created.
SurfaceBuffer *__fastcall SurfaceBuffer_Ctor(SurfaceBuffer *b);
SurfaceBuffer *__fastcall SurfaceBuffer_DeletingDtor(SurfaceBuffer *b, void *edx, u32 flags);  // 0x630700
void __fastcall SurfaceBuffer_Dtor(SurfaceBuffer *b);                                          // 0x630720
// 0x630730 (&out, &{width, height}, caps): releases the old one first; -1 when Surface_Create fails.
s32 *__fastcall SurfaceBuffer_Create(SurfaceBuffer *b, void *edx, s32 *out, const u32 *size, u32 caps);
void __fastcall SurfaceBuffer_Release(SurfaceBuffer *b);  // 0x630790
}
