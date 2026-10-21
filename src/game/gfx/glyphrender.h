// The glyph renderers (0x6b0430-0x6b4aa0): the classes the text layout's style holder picks from the table at 0x782d38 (its +0x54) to
// draw a glyph. The base (vtable 0x70a238: slot 0 the deleting destructor, slot 1 the draw call 0x6b4aa0) has four kinds, one global
// object each:
// - GlyphBlend (vtable 0x70a188, the object at 0xfb1efc): anti-aliased glyphs, the destination blended toward the colour;
// - GlyphSolid (vtable 0x70a1a8, 0xfb1ef0): the coverage thresholded or written as the pixel's alpha;
// - two patterned kinds (vtables 0x70a1c8 at 0xfb1ef4 and 0x70a1e8 at 0xfb1ef8) that tile a pattern surface.
// Slot 1 picks a span writer by the surface's pixel format: slot 2 for 1555, 3 for 565, 4 for 4444, 6 for 8888 (red at bit 16) and 7 for
// the byte-reversed 32-bit format (blue at bit 24); slot 5 is StoreThrough_ffffffff_6b4a00 (no writer).
//
// A span writer (thiscall, `this` unused; pops 0xc) takes (status, surface, span):
// - the surface: +0x18 the bits, +0x24 the pitch;
// - the span: +0 a point (x, y), where the glyph goes on the surface; +4 the glyph's coverage, 4 bits a pixel, the high nibble first;
//   +0xc a rectangle (left, top, right, bottom) of the glyph, in pixels; +0x10 the style, whose colour is the dword at +0x44 (blue in the
//   low byte, alpha in the high one); the byte at +0x14 the glyph's row length in pixels.
// With a colour alpha of 0 nothing is drawn. Otherwise the rectangle's (bottom - top) rows of (right - left) pixels each are walked, both
// counts unsigned (a negative one runs on, sic), from the nibble top * row length + left; after each row the nibble index moves on by
// left - right + row length (unsigned: a row length under the width runs off, sic). A pixel's coverage is its nibble, scaled by
// nibble * alpha / 255 for an alpha other than 0xff. *status = 0 at the end. Returns status.
// Reimplementation of 0x6b0430, 0x6b0450, 0x6b0730, 0x6b0760, 0x6b0c50, 0x6b1130, 0x6b15f0, 0x6b19f0, 0x6b1e00, 0x6b1e30, 0x6b2100,
// 0x6b21f0, 0x6b24b0, 0x6b2770, 0x6b2a30, 0x6b2a60, 0x6b2e20, 0x6b31e0, 0x6b3540, 0x6b3890, 0x6b3c20, 0x6b3c50, 0x6b3f40, 0x6b4140,
// 0x6b4420, 0x6b4700, 0x6b4a30, 0x6b4a70, 0x6b4aa0, 0x6b5340 and 0x6b5410. The names are unconfirmed.
#pragma once

#include "../../../re/types.h"
#include "../math/rect.h"

extern "C" {

// 0x6b0430 (thiscall (flags), pops 4): the deleting destructor of vtable 0x70a148 (Vtbl70a078_Set_6add30, then the object freed, Mem_Free,
// for flags bit 0). Returns this.
u8 *__fastcall Vtbl70a148_DeletingDtor(u8 *object, void *, u32 flags);

// 0x6b0450 (thiscall (flags), pops 4): the deleting destructor of vtable 0x70a160 (Wrap_Vtbl70a078_Set_6b4a50_6b0470, then freed for
// flags bit 0). Returns this.
u8 *__fastcall Vtbl70a160_DeletingDtor(u8 *object, void *, u32 flags);

// 0x6b4a30 (thiscall (flags), pops 4): the deleting destructor of vtable 0x70a210 (Vtbl70a078_Set_6b4a50, then freed for flags bit 0).
// Returns this.
u8 *__fastcall Vtbl70a210_DeletingDtor(u8 *object, void *, u32 flags);

// 0x6b4a70 (thiscall (flags), pops 4): the renderer base's deleting destructor (Vtbl70a258_Set_6b4a90, then freed for flags bit 0).
// Returns this.
u8 *__fastcall GlyphRenderer_DeletingDtor(u8 *renderer, void *, u32 flags);

// 0x6b5410 (thiscall (flags), pops 4): the deleting destructor of vtable 0x70a258, the one Vtbl70a258_Set_6b4a90 leaves (that vtable
// stored, then the object freed for flags bit 0). Returns this.
u8 *__fastcall Vtbl70a258_DeletingDtor(u8 *object, void *, u32 flags);

// 0x6b0730, 0x6b1e00, 0x6b2a30, 0x6b3c20 (thiscall (flags), pops 4): the four kinds' deleting destructors (their thunks to
// Vtbl70a258_Set_6b4a90, 0x6b0750, 0x6b1e20, 0x6b2a50, 0x6b3c40, then freed for flags bit 0). Return this.
u8 *__fastcall GlyphBlend_DeletingDtor(u8 *renderer, void *, u32 flags);
u8 *__fastcall GlyphSolid_DeletingDtor(u8 *renderer, void *, u32 flags);
u8 *__fastcall GlyphPattern1_DeletingDtor(u8 *renderer, void *, u32 flags);
u8 *__fastcall GlyphPattern2_DeletingDtor(u8 *renderer, void *, u32 flags);

// The draw call's calls to crt_atexit go through env (nullptr: the game's).
struct GlyphRenderEnv {
    s32(__cdecl *atExit)(void(__cdecl *fn)());   // crt_atexit (0x67a7d0)
};
void GlyphRender_SetEnv(const GlyphRenderEnv *env);

// 0x6b4aa0 (slot 1; thiscall (status, point, index, out, style), pops 0x14): draws glyph `index` of the style's font
// (TextStyle_GetFont, released through its slot 0 before returning) with its top left at point on the display's surface (the draw
// context's at 0xfa76e8). *status = -1 with no font (the release then faults on the null, sic), a paletted surface (+0x14 not 0), no
// glyph (Table_AddressPlus18 0) or nothing left inside the clip. The glyph's header (Table_Address: +0 the character, +8 the coded
// size, +0xc the coding, +0x10 / +0x12 the width and height, +0x14 / +0x15 the offset from point, signed bytes) is followed by its data.
// Coding bit 0: run-length coded nibbles (a nibble other than 0 stands for itself; 0, then a count and a nibble, for the nibble count
// times; 0, 0 ends) decoded into the shared buffer (0xfb4cd4, 0x800 bytes made on first use and its release 0x6b5560 handed to atexit,
// remade bigger for a larger coded size); bit 1: one bit a pixel (the high bit first), coded size * 8 of them, as the nibbles 0 / 0xf
// into the buffer (after bit 0's when both are set). A space (character 0x20) sets *out to the empty rectangle at point and *status 0.
// Otherwise the glyph is placed at point + its offset, clipped (GlyphRenderer_ClipSpan), *out set to the drawn rectangle (the placed
// point moved inside the clip's top left) and the span (that point, the decoded or raw nibbles, (width + 1) / 2, the visible part,
// the style, the width's low byte) handed to the span writer for the surface's format: slot 2 for 1555 / 555, 3 for 565, 4 for 4444,
// 6 for 8888 / 888, 7 for the byte-reversed 32-bit formats (PixelFormat_Equal), 5 when none matched; every check runs. *status = the
// dword the writer's return points to. Returns status.
s32 *__fastcall GlyphRenderer_Draw(u8 *renderer, void *, s32 *status, const s32 *point, u32 index, Rect *out, u8 *style);

// 0x6b5340 (thiscall (out, glyph, point, unused), pops 0x10; `this` unused): the part of the glyph's rectangle that shows when its
// top left goes at point: the placed rectangle clipped to the display's clip (Rect_Clip), moved back into the glyph's coordinates (its
// left and top moved on by how far the point lies left of / above the clip). Returns out.
Rect *__fastcall GlyphRenderer_ClipSpan(void *self, void *, Rect *out, const Rect *glyph, const s32 *point, u32 unused);

// GlyphBlend's span writers: each channel moves toward the colour's by (colour - old) * coverage / 15, the colour's channel taken at the
// pixel's precision, the result added back to the old pixel and masked to the channel. The 16-bit formats divide signed; the 32-bit ones
// unsigned (a channel darker than the old one wraps, sic).
// 0x6b0760: 1555. Green at the field's own scale (0x3e0, from (colour byte 1 & 0xf8) << 2), red at 0xf8 (the old one's (pixel >> 7) & 0xf8,
// the result shifted back by 7), blue at 0x1f (colour byte 0 >> 3); bit 15 kept.
s32 *__fastcall GlyphBlend_Span1555(void *self, void *, s32 *status, const u8 *surface, const u8 *span);
// 0x6b0c50: 565. Green at 0x7e0 (from (byte 1 & 0xfc) << 3), red at 0xf8 (the old one's (pixel >> 8) & 0xf8, shifted back by 8), blue at
// 0x1f.
s32 *__fastcall GlyphBlend_Span565(void *self, void *, s32 *status, const u8 *surface, const u8 *span);
// 0x6b1130: 4444. Red at 0xf00 ((byte 2 & 0xf0) << 4), green at 0xf0 (byte 1 & 0xf0), blue at 0xf (byte 0 >> 4); the alpha nibble kept.
s32 *__fastcall GlyphBlend_Span4444(void *self, void *, s32 *status, const u8 *surface, const u8 *span);
// 0x6b15f0: 8888 with red at bit 16, green at 8, blue at 0 (the colour's bytes 2, 1, 0); the high byte kept.
s32 *__fastcall GlyphBlend_Span8888(void *self, void *, s32 *status, const u8 *surface, const u8 *span);
// 0x6b19f0: 32 bits with blue at bit 24, green at 16, red at 8 (the colour's bytes 0, 1, 2); the low byte kept.
s32 *__fastcall GlyphBlend_SpanBgra(void *self, void *, s32 *status, const u8 *surface, const u8 *span);

// GlyphSolid's span writers: the colour packed to the format; pixels written over, the old ones not read.
// 0x6b1e30: 1555. A coverage with bit 3 set (8 or more) writes 0x8000 | the colour; others nothing.
s32 *__fastcall GlyphSolid_Span1555(void *self, void *, s32 *status, const u8 *surface, const u8 *span);
// 0x6b2100: 565. The whole rectangle filled with the colour (the alpha and the glyph not looked at, sic), its counts the low bytes of
// bottom - top and right - left.
s32 *__fastcall GlyphSolid_Span565(void *self, void *, s32 *status, const u8 *surface, const u8 *span);
// 0x6b21f0: 4444. A coverage other than 0 writes coverage << 12 | the colour.
s32 *__fastcall GlyphSolid_Span4444(void *self, void *, s32 *status, const u8 *surface, const u8 *span);
// 0x6b24b0: 8888 (red at bit 16). A coverage other than 0 writes coverage * 0x11 << 24 | the colour.
s32 *__fastcall GlyphSolid_Span8888(void *self, void *, s32 *status, const u8 *surface, const u8 *span);
// 0x6b2770: green at bit 16, red at 8; the packing ((byte 0 << 16 | byte 1) << 8 | byte 2) << 8 shifts blue out, so bits 24-31 stay 0
// (sic). A coverage other than 0 writes coverage * 0x11 | the colour.
s32 *__fastcall GlyphSolid_SpanBgra(void *self, void *, s32 *status, const u8 *surface, const u8 *span);

// The patterned kinds' span writers: the colour of each pixel is the pattern's, a 32-bit surface held in the style at +0x14 (+0x14..+0x28
// its format, +0x2c the bits, +0x30 the width, +0x34 the height, +0x38 the pitch) and tiled from the origin at +0x3c (x, y). The format
// must be gFormatArgb8888's (0xfaa280: the masks and bit count equal, the paletted flags both zero or both not), or *status = -1 and
// nothing is drawn. The pattern's column is (point x - origin x) % width and its row (point y - origin y) % height, both unsigned (a width
// or height of 0 faults, sic, the row's even with no rows to draw); the walk wraps both at the pattern's edge. The glyph is walked as
// GlyphBlend's, its coverage the bare nibble (the style's colour and alpha not read). *status = 0 at the end. Return status.
// 0x6b2a60, 0x6b2e20, 0x6b3540: GlyphPattern1 (vtable 0x70a1c8), GlyphBlend's 1555, 565 and 8888 blends toward the pattern's pixel.
s32 *__fastcall GlyphPattern1_Span1555(void *self, void *, s32 *status, const u8 *surface, const u8 *span);
s32 *__fastcall GlyphPattern1_Span565(void *self, void *, s32 *status, const u8 *surface, const u8 *span);
s32 *__fastcall GlyphPattern1_Span8888(void *self, void *, s32 *status, const u8 *surface, const u8 *span);
// 0x6b31e0: 4444 as GlyphBlend's but the divisions unsigned (sic).
s32 *__fastcall GlyphPattern1_Span4444(void *self, void *, s32 *status, const u8 *surface, const u8 *span);
// 0x6b3890: blue at bit 24, green at 16, red at 8 (unsigned, as GlyphBlend's); the old pixel's top byte ORed into the new one's and
// the low byte cleared (sic).
s32 *__fastcall GlyphPattern1_SpanBgra(void *self, void *, s32 *status, const u8 *surface, const u8 *span);
// GlyphPattern2 (vtable 0x70a1e8): the pattern's pixel written over.
// 0x6b3c50: 1555 with a coverage's bit 0 set: the pixel packed to 555 | 0x1000 (bit 12, in red; the alpha bit not set, sic).
s32 *__fastcall GlyphPattern2_Span1555(void *self, void *, s32 *status, const u8 *surface, const u8 *span);
// 0x6b3f40: 565. The whole rectangle filled (the glyph not walked), each pixel packed to 555 (sic).
s32 *__fastcall GlyphPattern2_Span565(void *self, void *, s32 *status, const u8 *surface, const u8 *span);
// 0x6b4140, 0x6b4420: GlyphSolid's 4444 and 8888 with the pattern's pixel as the colour.
s32 *__fastcall GlyphPattern2_Span4444(void *self, void *, s32 *status, const u8 *surface, const u8 *span);
s32 *__fastcall GlyphPattern2_Span8888(void *self, void *, s32 *status, const u8 *surface, const u8 *span);
// 0x6b4700: a coverage other than 0 writes the pattern's bytes 0, 1, 2 at bits 24, 16, 8 | coverage * 0x11.
s32 *__fastcall GlyphPattern2_SpanBgra(void *self, void *, s32 *status, const u8 *surface, const u8 *span);
}
