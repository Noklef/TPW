// A framed gauge (a bar with three marks each way, drawn in four colours; set up at 0x4806ff
// and drawn by 0x47fc00 / 0x47fc50): reimplementation of its plain members 0x47faa0-0x47fbe0.
// +0 width, +4 height, +8 left, +0xc top, +0x10 right, +0x14 bottom; +0x18 / +0x1c, +0x20 /
// +0x24, +0x28 / +0x2c three marks (x / y); four 5-byte colours (rgb, palette index) at +0x30,
// +0x35, +0x3a and +0x3f.
#pragma once

#include "../../../re/types.h"
#include "draw.h"

extern "C" {

// 0x47faa0: all zero (the colours bytewise); returns gauge.
u8 *__fastcall Gauge_Init(u8 *gauge);

// 0x47fb10 / 0x47fb30: the three marks' x (+0x18, +0x20, +0x28) / y (+0x1c, +0x24, +0x2c).
void __fastcall Gauge_SetMarksX(u8 *gauge, void *, u32 a, u32 b, u32 c);
void __fastcall Gauge_SetMarksY(u8 *gauge, void *, u32 a, u32 b, u32 c);

// 0x47fb50: the frame, and its width (right - left) and height (bottom - top).
void __fastcall Gauge_SetRect(u8 *gauge, void *, s32 left, s32 top, s32 right, s32 bottom);

// 0x47fb80, 0x47fba0, 0x47fbc0, 0x47fbe0: colour 0-3 (its rgb and index; 5 bytes).
void __fastcall Gauge_SetColour0(u8 *gauge, void *, DrawColour c);
void __fastcall Gauge_SetColour1(u8 *gauge, void *, DrawColour c);
void __fastcall Gauge_SetColour2(u8 *gauge, void *, DrawColour c);
void __fastcall Gauge_SetColour3(u8 *gauge, void *, DrawColour c);

// 0x47fc00 (thiscall; name unconfirmed): the frame (+8 to +0x14) filled in colour 3 (Display_BoxFill).
void __fastcall Gauge_DrawBack(const u8 *gauge);

// 0x47fc50 (thiscall; name unconfirmed): the axes: a horizontal line in colour 0 from left - 1, right + 1 long (sic: not the width), at
// bottom + 1, or when the first mark's y (+0x1c) is negative and the second's (+0x24) is above it, at bottom + height * y0 / (y1 - y0)
// (32-bit, truncated); then a vertical one in colour 1 from (left - 1, top - 1), bottom + 1 long (Display_HLine, Display_VLine).
void __fastcall Gauge_DrawAxes(const u8 *gauge);

// 0x4806a0 (cdecl; name unconfirmed): sets a gauge up for a window whose s16 left, top, right, bottom are at +4..+0xa of `window`: the frame
// (4, 4) to ((right - left) * s16 [0xfaa5c4] / 2048 - 20, (bottom - top) * s16 [0xfaa5c0] / 1536 - 20) (32-bit products, divisions truncated);
// colours 0 and 1 green (0, 200, 0), 3 black, 2 dark green (0, 100, 0), each with its nearest entry of the game's palette 0xfa71f8 (Palette_Nearest),
// set in the order 0, 1, 3, 2; then the marks' y (y0..y2) and x (x0..x2).
void __cdecl Gauge_Setup(u8 *gauge, const u8 *window, u32 y0, u32 y1, u32 y2, u32 x0, u32 x1, u32 x2);
}
