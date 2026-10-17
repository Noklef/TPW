// The render view's screen and window geometry: reimplementation of 0x582eb0-0x5832dc (thiscall on the view object at 0x8bcbc8, whose
// first dword is the render flags; x87 at the game's 24-bit precision). Its fields, as the code uses them:
//   +0x04 the screen's aspect (height / width), +0x08 / +0x0c the screen's width and height, +0x10 / +0x14 the screen's half sizes
//   ((size - 0.999) * 0.5), +0x1c the third argument of SetScreen (the depth, unconfirmed);
//   +0x20 / +0x24 the window's width and height, +0x30 / +0x34 its centre, +0x38 / +0x3c its half sizes, +0x28 / +0x2c the centre
//   moved by 0.999 * 0.25 (the pixel centre, unconfirmed);
//   +0x40, +0x4c, +0x50, +0x54 the projection's inputs and +0x60-+0x70 its outputs (SetProjection);
//   +0x8c8-+0x8dc the depth range (SetDepthRange).
// The names are unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x582eb0 (width, height, depth; pops 0xc): the screen's sizes stored, its aspect (height / width of the two integers, rounded once)
// and half sizes made, and the window set to the whole screen. With the driver kind 0x8bd524 == 4 the flag 0x10000 is cleared.
void __fastcall View_SetScreen(u8 *view, void *, s32 width, s32 height, u32 depth);

// 0x582f70 (x, y, width, height: floats, the window in fractions of the screen; pops 0x10): the window's sizes are the screen's
// times the fractions, halved and __ftol'd (low half); its centre comes from x + 1 and 1 - (height + y), each scaled, halved and
// __ftol'd again (sic: a truncated offset taken off a float).
void __fastcall View_SetWindow(u8 *view, void *, float x, float y, float width, float height);

// 0x583080: the window set to the whole screen (the screen's sizes and half sizes copied).
void __fastcall View_ResetWindow(u8 *view);

// 0x583220 (unused, a, b: pops 0xc): +0x60 = a; with q = a / b and d = (+0x50 - +0x54) / (q - 1): +0x64 = d * +0x40,
// +0x68 = (+0x54 - d) * +0x40, +0x6c = (+0x50 - +0x4c) / a * +0x40 and +0x70 = +0x40 * +0x4c.
void __fastcall View_SetProjection(u8 *view, void *, u32 unused, float a, float b);

// 0x583280 (near, far; pops 8): +0x8c8 = near, +0x8cc = far, +0x8d0 = 1 / (far - near), +0x8d4 / +0x8d8 their squares and +0x8dc
// the square of +0x8d0.
void __fastcall View_SetDepthRange(u8 *view, void *, float near_, float far_);
}
