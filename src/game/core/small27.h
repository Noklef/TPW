// Three small helpers: a thing looked up by handle, a map cell handle made from its coordinates and four global-memory buffers freed: reimplementation of 0x4cd300, 0x4cd4e0
// and 0x413340. Named by what they do until the callers are known.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x4cd300 (stdcall; handle, pops 4): the thing the slot (handle & 0xffff) of the thing table at 0x7cfb90 holds (20-byte slots; the debug message of the original is gone).
u8 *__stdcall Thing_ByHandle(u32 handle);

// 0x4cd4e0 (cdecl; out, x, z): the word *out = (z << 7) + x + 1. Returns out.
u16 *__cdecl Cell_MakeHandle(u16 *out, u32 x, u32 z);

// 0x413340 (thiscall): the four buffers the dwords from +0x4f4 point to are freed (Mem_GlobalFree) and the dwords cleared, except those that are 0 or the shared buffer at
// 0x6fd880.
void __fastcall Obj_FreeFourBuffers(u8 *object);

}  // extern "C"
