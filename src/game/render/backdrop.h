// The tiled backdrop's setters and its tiles' uncaching: reimplementation of 0x5777a0, 0x577940, 0x577970 and 0x577990.
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

// 0x577990 (thiscall; x0, y0, x1, y1 in pixels; name unconfirmed): the tiles (x0 / 64 .. x1 / 64 by y0 / 64 .. y1 / 64, divided
// toward 0, both ends taken) taken out of the texture cache: TexCache_UnlinkItem on the cache at [0x8c4b44] with the tile
// (0x60 bytes each from +0x1c) at y * the row's tiles (+0x10) + x + the first tile (+0x48).
void __fastcall Backdrop_UncacheRect(u8 *backdrop, void *, s32 x0, s32 y0, s32 x1, s32 y1);
}
