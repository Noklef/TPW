// Antialiased lines on a surface's pixels: reimplementation of 0x5edad0, 0x5ee580, 0x5ef040 and 0x5efb10, one per pixel size (1, 2, 3
// and 4 bytes), which the park map's line drawing (0x5f2fd0) picks from the surface's bit count. The names are unconfirmed.
//
// Each (cdecl; an unused first argument, the surface's info, the clip rectangle {left, top, right, bottom} (right and bottom
// exclusive), the colour, the two end points {x, y}) converts the colour to a pixel (Color_ToPixel) and steps along the major axis
// from the first point (the points swapped when it runs backwards, sic: only the major coordinate's end is kept, the minor one's start
// is the other point's) Bresenham-style, drawing three pixels per step: the line's own and its two neighbours across it. Each one's
// distance from the line, the error term d (0 to start) over twice the length (x87 at single precision: 1.0 / (2 sqrt(dx^2 + dy^2)),
// the minor neighbours at 2 major / (2 length) -+ that), is compared as a double: inside the clip rectangle and not beyond 1.0 (a NaN
// passes) the pixel takes (1.0 - |distance|) * 255.0 (__ftol's low byte) of the colour: per red, green and blue mask the old pixel's
// field times 255 - a plus the colour's times a, + itself >> 8, + 1, >> 8, masked; the alpha mask's bits kept from the old pixel. The
// old pixel is read as a dword whatever the size; the new one's low bytes are written.
#pragma once

#include "../../../re/types.h"
#include "color.h"
#include "surface.h"

extern "C" {

void __cdecl AaLine_Draw8(void *unused, const SurfaceInfo *surface, const s32 *clip, Color *colour, const s32 *from, const s32 *to);
void __cdecl AaLine_Draw16(void *unused, const SurfaceInfo *surface, const s32 *clip, Color *colour, const s32 *from, const s32 *to);
void __cdecl AaLine_Draw24(void *unused, const SurfaceInfo *surface, const s32 *clip, Color *colour, const s32 *from, const s32 *to);
void __cdecl AaLine_Draw32(void *unused, const SurfaceInfo *surface, const s32 *clip, Color *colour, const s32 *from, const s32 *to);
}
