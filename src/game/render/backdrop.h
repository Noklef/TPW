// The tiled backdrop's setters: reimplementation of 0x5777a0, 0x577940 and 0x577970.
//
// The backdrop is the object at 0x87aee8: an image cut into 64x64 tiles (made by 0x5777b0, its
// width at +4 and height at +8), drawn by the renderer when its state flags have 0x2000
// (0x576c0a, 0x576cd7; the signs set 0x2821 around theirs). +0x20..+0x2c is the rectangle it is
// drawn into (x0, y0, x1, y1, floats) and +0x30..+0x3c the part of the image drawn (0x577b3f).
// +0x5c is read when drawing (0x577ea0).
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x5777a0 (thiscall): +0x5c = 0.
void __fastcall Backdrop_Clear5c(u8 *backdrop);

// 0x577940 (thiscall; x0, y0, x1, y1, floats moved as dwords): +0x20..+0x2c, and flag 2 in +0.
void __fastcall Backdrop_SetRect(u8 *backdrop, void *, u32 x0, u32 y0, u32 x1, u32 y1);

// 0x577970 (thiscall; x0, y0, x1, y1): the image's part drawn, +0x30..+0x3c.
void __fastcall Backdrop_SetSourceRect(u8 *backdrop, void *, u32 x0, u32 y0, u32 x1, u32 y1);
}
