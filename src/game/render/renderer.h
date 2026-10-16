// The renderer object's accessors: reimplementation of 0x576260, 0x5763f0 and 0x576400.
//
// The renderer is the object at [0x87a6d0] (built at 0x576162 with a pool of 0x4800 8-byte nodes
// at +0; its frame buffer, +0x38 wide by +0x3c high, at +0x10). +0x48 starts its render state
// (the flags, 0x1f31 when the buffer is made, 0x576379), which callers save, replace (0x21,
// 0x2821) and put back around their drawing (0x4a3aa0, 0x5f0bad). +0x78 is a function called
// when the display device has been restored since the last frame (its generation, 0x8bcc40,
// differs from +0x74; 0x576c26).
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x576260 (thiscall; fn): +0x78 = fn, the device restore callback (0x54e2b0, or 0).
void __fastcall Renderer_SetRestoreCallback(u8 *renderer, void *, u32 fn);

// 0x5763f0 (thiscall; flags): +0x48 = flags.
void __fastcall Renderer_SetStateFlags(u8 *renderer, void *, u32 flags);

// 0x576400 (thiscall): +0x48.
u32 __fastcall Renderer_StateFlags(const u8 *renderer);
}
