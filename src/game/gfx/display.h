// The drawing front end (0x5f9780-0x5f9e64): the Bullfrog library's lbDisplay calls the game
// draws through. They test the shape's bounding box against the clip rectangle and hand it to
// the current depth's table (see draw.h); set the window and clip rectangles; and point the
// context at a surface, choosing the table by its bit count.
//
// The window is a rectangle of the surface (all of it by default); drawing coordinates are
// relative to its top left, and the clip rectangle is in those coordinates, cut to the surface
// and to the window.
//
// The originals' quirks are kept: the bits pointer moves by bitCount >> 3 bytes a pixel (no
// rounding up); a surface that is neither 16, 24 nor 32 bits draws with the 8-bit table; and the
// colour reaches the table with only its rgb and index copied (its padding is whatever was
// on the stack).
#pragma once

#include "../../../re/types.h"
#include "../math/rect.h"
#include "draw.h"
#include "surface.h"

extern "C" {
// 0x5f9780: whether r (in either order) touches the clip rectangle, edges inclusive.
u32 __cdecl Display_IsVisible(const Rect *r);
void __cdecl Display_Pixel(s32 x, s32 y, DrawColour c);  // 0x5f97c0: slot 0, untested
// 0x5f97f0: slot 7, when the bounding box is visible.
void __cdecl Display_Line(s32 x1, s32 y1, s32 x2, s32 y2, DrawColour c);
void __cdecl Display_HLine(s32 x, s32 y, s32 length, DrawColour c);  // 0x5f9860: slot 8
void __cdecl Display_VLine(s32 x, s32 y, s32 length, DrawColour c);  // 0x5f98d0: slot 9
// 0x5f9940: slot 10, when the bounding box is visible.
void __cdecl Display_Triangle(s32 x1, s32 y1, s32 x2, s32 y2, s32 x3, s32 y3, DrawColour c);
void __cdecl Display_FillRect(const Rect *r, DrawColour c);  // 0x5f9a30: r, or 0 for the clip
void __cdecl Display_BoxFill(const Rect *r, DrawColour c);   // 0x5f9a80: slot 3, when visible
// 0x5f9ac0: the window (r, or 0 for all of the surface; put in order), the bits at its top left,
// then the clip rectangle reset to all of it.
void __cdecl Display_SetWindow(const Rect *r);
// 0x5f9b60: the clip rectangle (r, or 0 for all of the window), cut to the window and the
// surface; nothing left is {0, 0, 0, 0}.
void __cdecl Display_SetClip(const Rect *r);
Rect *__cdecl Display_GetClip();  // 0x5f9ca0
// 0x5f9cb0: draw on i (0: none), all of it; the table for its bit count and its channel shifts.
void __cdecl Display_SetSurfaceInfo(SurfaceInfo *i);
// 0x5f9d90: the same for a surface (0: none), after refreshing its description, taking the draw
// flags, ghost and remap tables and +8 from it as well.
void __cdecl Display_SetSurface(Surface *s);
}

// The context goes through state (nullptr: the game's, 0xfa76b8).
void Display_SetEnv(DrawContext *state);
