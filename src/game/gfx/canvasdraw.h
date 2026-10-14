// The canvas primitives: pixel, spans, line, outline, filled rectangle, triangle, mask and sprite
// (CanvasDriver slots 2-5 and 7-11; thiscall on the driver, which only the line, outline and
// triangle use). Each depth has its own copy of the code; the pixel and spans are at
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
// - The 24-bit blended horizontal span and filled rectangle never advance along the row: each
//   row blends its first pixel over and over.
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
    Rect *(__fastcall *clip)(const Rect *, void *, Rect *, const Rect *);  // Rect_Clip 0x62f810
};
// Slot 10's run-length mask.
struct CanvasMask {
    const u8 *codes;
    u16 width, height;
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
// Slot 4: (surface, clip, flags, x0, y0, x1, y1, colour): a vertical or horizontal line through
// slot 3 or 2 (the far end excluded), otherwise clipped and drawn by Bresenham, both ends drawn.
void __fastcall Canvas8_Line(CanvasDriver *d, void *edx, const SurfaceInfo *s, const Rect *clip, u32 flags, s32 x0,
                             s32 y0, s32 x1, s32 y1, Color *colour);
void __fastcall Canvas16_Line(CanvasDriver *d, void *edx, const SurfaceInfo *s, const Rect *clip, u32 flags, s32 x0,
                              s32 y0, s32 x1, s32 y1, Color *colour);
void __fastcall Canvas24_Line(CanvasDriver *d, void *edx, const SurfaceInfo *s, const Rect *clip, u32 flags, s32 x0,
                              s32 y0, s32 x1, s32 y1, Color *colour);
void __fastcall Canvas32_Line(CanvasDriver *d, void *edx, const SurfaceInfo *s, const Rect *clip, u32 flags, s32 x0,
                              s32 y0, s32 x1, s32 y1, Color *colour);
// Slot 9: (surface, clip, flags, x0, y0, x1, y1, x2, y2, colour). With flag 4 the three edges
// through slot 4, each vertex inside the clip put back as the first line to reach it left it;
// otherwise the vertices sorted by y and rows y0 to y2 - 1 filled through slot 2 between 16.16
// edges (the right end excluded).
void __fastcall Canvas8_Triangle(CanvasDriver *d, void *edx, const SurfaceInfo *s, const Rect *clip, u32 flags,
                                 s32 x0, s32 y0, s32 x1, s32 y1, s32 x2, s32 y2, Color *colour);
void __fastcall Canvas16_Triangle(CanvasDriver *d, void *edx, const SurfaceInfo *s, const Rect *clip, u32 flags,
                                  s32 x0, s32 y0, s32 x1, s32 y1, s32 x2, s32 y2, Color *colour);
void __fastcall Canvas24_Triangle(CanvasDriver *d, void *edx, const SurfaceInfo *s, const Rect *clip, u32 flags,
                                  s32 x0, s32 y0, s32 x1, s32 y1, s32 x2, s32 y2, Color *colour);
void __fastcall Canvas32_Triangle(CanvasDriver *d, void *edx, const SurfaceInfo *s, const Rect *clip, u32 flags,
                                  s32 x0, s32 y0, s32 x1, s32 y1, s32 x2, s32 y2, Color *colour);
// Slot 10: (surface, clip, flags, x, y, &mask, colour): the mask's rows of codes (n < 0 skips -n
// pixels, n > 0 draws n pixels of the colour and is followed by n bytes it ignores, 0 ends the
// row) drawn clipped, mirrored across by flag 1 and up and down by flag 2. Never blended.
// - Flipped across but not clipped across it draws forwards from the right end, and each skip
//   also passes the code after it.
// - Clipped across, a run the left edge cuts is dropped when the byte where its drawing would
//   resume is 0 (and the row is then taken to end there).
void __fastcall Canvas8_Mask(CanvasDriver *d, void *edx, const SurfaceInfo *s, const Rect *clip, u32 flags, s32 x,
                             s32 y, const CanvasMask *mask, Color *colour);
void __fastcall Canvas16_Mask(CanvasDriver *d, void *edx, const SurfaceInfo *s, const Rect *clip, u32 flags, s32 x,
                              s32 y, const CanvasMask *mask, Color *colour);
void __fastcall Canvas24_Mask(CanvasDriver *d, void *edx, const SurfaceInfo *s, const Rect *clip, u32 flags, s32 x,
                              s32 y, const CanvasMask *mask, Color *colour);
void __fastcall Canvas32_Mask(CanvasDriver *d, void *edx, const SurfaceInfo *s, const Rect *clip, u32 flags, s32 x,
                              s32 y, const CanvasMask *mask, Color *colour);
// Slot 11: (surface, clip, flags, table, x, y, &sprite): the same codes, each run's bytes indices
// into the table of pixels, drawn clipped and flipped likewise (correctly, but for the 24-bit
// code stepping 4 bytes a pixel).
void __fastcall Canvas8_Sprite(CanvasDriver *d, void *edx, const SurfaceInfo *s, const Rect *clip, u32 flags,
                               const u32 *table, s32 x, s32 y, const CanvasMask *sprite);
void __fastcall Canvas16_Sprite(CanvasDriver *d, void *edx, const SurfaceInfo *s, const Rect *clip, u32 flags,
                                const u32 *table, s32 x, s32 y, const CanvasMask *sprite);
void __fastcall Canvas24_Sprite(CanvasDriver *d, void *edx, const SurfaceInfo *s, const Rect *clip, u32 flags,
                                const u32 *table, s32 x, s32 y, const CanvasMask *sprite);
void __fastcall Canvas32_Sprite(CanvasDriver *d, void *edx, const SurfaceInfo *s, const Rect *clip, u32 flags,
                                const u32 *table, s32 x, s32 y, const CanvasMask *sprite);
// Slot 8: (surface, clip, flags, &rect, colour): the rectangle, ordered and clipped, filled.
void __fastcall Canvas8_FillRect(CanvasDriver *d, void *edx, const SurfaceInfo *s, const Rect *clip, u32 flags,
                                 const Rect *rect, Color *colour);
void __fastcall Canvas16_FillRect(CanvasDriver *d, void *edx, const SurfaceInfo *s, const Rect *clip, u32 flags,
                                  const Rect *rect, Color *colour);
void __fastcall Canvas24_FillRect(CanvasDriver *d, void *edx, const SurfaceInfo *s, const Rect *clip, u32 flags,
                                  const Rect *rect, Color *colour);
void __fastcall Canvas32_FillRect(CanvasDriver *d, void *edx, const SurfaceInfo *s, const Rect *clip, u32 flags,
                                  const Rect *rect, Color *colour);
// Slot 7 (all depths): the rectangle's edges through the driver's slots 2 and 3, the bottom and
// right ones a pixel inside; nothing when it misses the clip.
void __fastcall Canvas_Outline(CanvasDriver *d, void *edx, const SurfaceInfo *s, const Rect *clip, u32 flags,
                               const Rect *rect, Color *colour);
}
