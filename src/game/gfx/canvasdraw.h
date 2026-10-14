// The canvas primitives' pixel, spans and outline (CanvasDriver slots 2, 3, 5 and 7; thiscall on
// the driver, which only the outline uses). Each depth has its own copy of the pixel and span code
// (8 bits 0x601a00-0x601ed9, 16 bits 0x608140-0x608421, 24 bits 0x60ef50-0x60f407, 32 bits
// 0x615250-0x6156b6); the outline (0x6025c0) is shared. They take the surface, the clip rectangle
// and the canvas's flags: with neither 8 nor 0x10 the colour's pixel is written, otherwise each
// channel is blended towards it by the colour's alpha (0xff minus it with 0x10) through the
// surface's masks, the destination's alpha-mask bits kept.
//
// The originals' quirks are kept:
// - The pixel blends with ((mask - (mask & dst)) & src) where the spans use ((mask & src) -
//   (mask & dst)): mask bits set in the destination never reach the colour.
// - The 24-bit code reads the destination as a whole dword (a byte past the pixel) and the 24-
//   and 32-bit code uses the colour's whole pixel, so bits above the pixel can shift down into it.
// - The 24-bit blended horizontal span never advances: it blends its first pixel len times.
// - A negative length runs from x + len; the clip is tested before the span is clipped to it.
#pragma once

#include "../../../re/types.h"
#include "../math/rect.h"
#include "canvas.h"
#include "color.h"
#include "surface.h"

struct CanvasDrawEnv {
    u32(__fastcall *toPixel)(Color *, void *, const PixelFormat *);  // Color_ToPixel 0x5fd050
    void(__fastcall *order)(Rect *);                                 // Rect_Order 0x62f940
    u32(__fastcall *overlaps)(const Rect *, void *, const Rect *);   // Rect_Overlaps 0x62f890
};
// nullptr restores the game's.
void CanvasDraw_SetEnv(const CanvasDrawEnv *e);

extern "C" {
// Slot 5: (surface, clip, flags, x, y, colour).
void __fastcall Canvas8_Pixel(CanvasDriver *d, void *edx, const SurfaceInfo *s, const Rect *clip, u32 flags, s32 x,
                              s32 y, Color *colour);
void __fastcall Canvas16_Pixel(CanvasDriver *d, void *edx, const SurfaceInfo *s, const Rect *clip, u32 flags,
                               s32 x, s32 y, Color *colour);
void __fastcall Canvas24_Pixel(CanvasDriver *d, void *edx, const SurfaceInfo *s, const Rect *clip, u32 flags,
                               s32 x, s32 y, Color *colour);
void __fastcall Canvas32_Pixel(CanvasDriver *d, void *edx, const SurfaceInfo *s, const Rect *clip, u32 flags,
                               s32 x, s32 y, Color *colour);
// Slot 2: (surface, clip, flags, x, y, len, colour), len pixels rightwards.
void __fastcall Canvas8_HSpan(CanvasDriver *d, void *edx, const SurfaceInfo *s, const Rect *clip, u32 flags, s32 x,
                              s32 y, s32 len, Color *colour);
void __fastcall Canvas16_HSpan(CanvasDriver *d, void *edx, const SurfaceInfo *s, const Rect *clip, u32 flags,
                               s32 x, s32 y, s32 len, Color *colour);
void __fastcall Canvas24_HSpan(CanvasDriver *d, void *edx, const SurfaceInfo *s, const Rect *clip, u32 flags,
                               s32 x, s32 y, s32 len, Color *colour);
void __fastcall Canvas32_HSpan(CanvasDriver *d, void *edx, const SurfaceInfo *s, const Rect *clip, u32 flags,
                               s32 x, s32 y, s32 len, Color *colour);
// Slot 3: the same, downwards.
void __fastcall Canvas8_VSpan(CanvasDriver *d, void *edx, const SurfaceInfo *s, const Rect *clip, u32 flags, s32 x,
                              s32 y, s32 len, Color *colour);
void __fastcall Canvas16_VSpan(CanvasDriver *d, void *edx, const SurfaceInfo *s, const Rect *clip, u32 flags,
                               s32 x, s32 y, s32 len, Color *colour);
void __fastcall Canvas24_VSpan(CanvasDriver *d, void *edx, const SurfaceInfo *s, const Rect *clip, u32 flags,
                               s32 x, s32 y, s32 len, Color *colour);
void __fastcall Canvas32_VSpan(CanvasDriver *d, void *edx, const SurfaceInfo *s, const Rect *clip, u32 flags,
                               s32 x, s32 y, s32 len, Color *colour);
// Slot 7 (all depths): the rectangle's edges through the driver's slots 2 and 3, the bottom and
// right ones a pixel inside; nothing when it misses the clip.
void __fastcall Canvas_Outline(CanvasDriver *d, void *edx, const SurfaceInfo *s, const Rect *clip, u32 flags,
                               const Rect *rect, Color *colour);
}
