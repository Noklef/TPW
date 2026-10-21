// The sprite frames' and images' constructors, their textures' release, a frame's quad and a frame set's texture flags: reimplementation
// of 0x5890e0, 0x588d70, 0x588320, 0x5882e0, 0x588ba0, 0x588c20, 0x588ca0, 0x588350, 0x588230, 0x588d90 and 0x588e40.
//
// A sprite frame (0x48 bytes; a font's at +0x54, 0x48f18b; the decoded textures', 0x56b4f5) is
// readied for drawing by 0x588ef0, which queues it in the sprite queue (spritequeue.h): +0x10 its
// flags, +0x28 its image, +0x34 / +0x36 (s16) the size it was last made at (0x588d90). Its image
// (0x18 bytes; the font's at +0x9c) has two pointers and six u16 sizes from +0xc. A frame set
// (0x587db0) is a count (+0) and its images (+4), each image's +0 its texture (texture.h).
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x588d70 (thiscall): +0x10, +0x28, +0x30 and +0x34 = 0, +0x24 = -1, +0x2c = 1.0f; returns the
// frame.
u8 *__fastcall SpriteFrame_Init(u8 *frame);

// 0x588940 (thiscall; width, height; name unconfirmed): +0xc / +0xe = the sizes' low words; +4 = a tracked block (TrackedGlobal_Alloc,
// core/gameheap) of the current format's pixel size (0x701fe0 indexed by [0x8bd554]) times both: 1. Not got: the error (2, 0xb0754000)
// pushed (EngineErrors_Push), 0.
u32 __fastcall SpriteImage_AllocPixels(u8 *image, void *, u32 width, u32 height);

// 0x588410 (thiscall (pixels, palette, source, width, height, texture, flags), pops 0x1c; name unconfirmed): +0 = the texture given,
// or else a new one (a block of 100: a count of 1, one texture built by CRT_VectorVbaseCtorIterator with Texture_Ctor; not got: the
// error (2, 0xb0754000) pushed and 0). The texture's +0x40 = source (0: pixels), +0x3c = palette, the flags 0x1000040 (0x1000440 with a
// palette) | flags | 0x100, its size set (Texture_SetSize) and +0x34 = the width's low word. Returns 1.
u32 __fastcall SpriteImage_SetTexture(u8 *image, void *, u32 pixels, u32 palette, u32 source, u32 width, u32 height, u8 *texture,
                                      u32 flags);

// 0x588510 (thiscall (stream, texture), pops 8; called at 0x5880c3; name unconfirmed): a WCT image read for the sprite image: a new
// one (GameHeap_Alloc(0x30), Obj_CtorVirtual564ba0 most derived; not got: the error (2, 0xb0754000) pushed, 0) read from the stream
// (WctImage_Read, its result not looked at); the words of the header it leaves at 0x879a3c, 0x879a3e, 0x879a40, 0x879a42, 0x879a44 and
// 0x879a48 become the image's +0xc..+0x16; +8 = +0xc / +0xe (x87, 24 bits) or [0x702034] (0) for a +0xe of 0; then
// SpriteImage_SetTexture(0, 0, the WCT image's virtual base, +0xc, +0xe, texture, 0x10200000): 1, or when that fails the WCT image
// deleted through its virtual base's slot 0 (1) and 0. (The original's __try frame only guards the new.)
u32 __fastcall SpriteImage_LoadWct(u8 *image, void *, u8 *stream, u8 *texture);

// 0x588660 (thiscall (stream, language, texture), pops 0xc; called at 0x587f9f; name unconfirmed): the same for a run-length image
// (GameHeap_Alloc(0x1c), Obj_CtorVirtual5646c0): from the stream (Object_CallSlot8Product, count 1) a dword size, the words +0xc,
// +0xe, +0x10, +0x12 and two dwords whose low words become +0x14 / +0x16. The size and the two dwords are read into stack slots that
// held the block and the stream argument, which is what they keep where the stream writes nothing (kept here). Then
// RleImage_ReadCodes(stream, size) (its result not looked at), Fn_564730(+0xc, +0xe) and Language_SetId(language) on it, +8 and the
// texture as 0x588510's (an RLE image's virtual base as the source).
u32 __fastcall SpriteImage_LoadRle(u8 *image, void *, u8 *stream, u32 language, u8 *texture);

// 0x5889b0 (thiscall (x, y), pops 8; name unconfirmed): the image's width and height (u16 +0x10, +0x12) cut into tiles of the texture
// cache's size ([0x8c4b44] +0x18f0, signed divisions rounding up): the textures' array (a count-prefixed block of 0x60 each, built by
// CRT_VectorVbaseCtorIterator with Texture_Ctor) at +0 (not got: 0 there and 0 returned), the count's low word at +0x16. Row by row,
// each texture's flags = 0x5180200, its size the tile's (the last column's and row's ((tile - 1) & (size - 1)) + 1), +0x34 = the
// row width (u16 +0xc) and +0x40 = the pixels (+4) + ((y + row's offset) * row width + x + column's offset) * the format's pixel size
// (0x701fe0 indexed by [0x8bd554]). Returns 1.
u32 __fastcall SpriteImage_MakeTextures(u8 *image, void *, u32 x, u32 y);

// 0x588320 (thiscall): all 0x18 bytes but +8 cleared; returns the image.
u8 *__fastcall SpriteImage_Init(u8 *image);

// 0x5882e0 (thiscall): each image's texture gets flag 0x200 and loses 0x100 (Sprite_LoadEsp,
// Sprites_UseGroundFrame, Sprites_UseParkFrames). The count is re-read after each store.
void __fastcall SpriteFrames_MarkTextures(u8 *set);

// The image's textures (+0, an array of count-prefixed 0x60-byte textures, the count of them as a u16 at +0x16) are freed by these the
// same way: each texture with flag 0x1000 has its cache item unlinked (TexCache_UnlinkItem on the cache at 0x8c4b44), and after each the
// FIRST texture's +0x40 is cleared (sic: never the others'); then the array (when set) is destroyed (CRT_VectorDtorIterator with 0x590a70,
// Texture_Release) and freed 4 bytes below, and +0 and +0x16 cleared.

// 0x588ba0 (thiscall; name unconfirmed): the textures freed when the count is not 0 (the array not checked). Returns 1.
u32 __fastcall SpriteImage_FreeTextures(u8 *image);

// 0x588c20 (thiscall; name unconfirmed): the same when the array is set and the count not 0. Returns 1.
u32 __fastcall SpriteImage_FreeTexturesIfSet(u8 *image);

// 0x588ca0 (thiscall; name unconfirmed): with pixels at +4: the textures freed as SpriteImage_FreeTexturesIfSet does, then the pixels
// (TrackedGlobal_Free, core/gameheap) and +4 cleared. Without: when the array is set, Texture_Release on its first texture, then the array
// destroyed and freed (when still set) and +0 cleared (+0x16 kept). Returns 1.
u32 __fastcall SpriteImage_Release(u8 *image);

// 0x588350 (thiscall; name unconfirmed): the same as SpriteImage_Release, returning nothing (an image array's destructor).
void __fastcall SpriteImage_Dtor(u8 *image);

// 0x588230 (thiscall; name unconfirmed): a frame set's (+0 count, +4 its images, +8 its texture array, +0xc a tracked block) parts
// freed: the textures (when set) destroyed as one array (Texture_DestroyArray) and +8 cleared; with images, the first count images'
// +0 cleared (the count signed, re-read each time; sic: past the images' own count it writes on) and the images (when still set)
// destroyed with 0x588350 and freed, +4 cleared; the tracked block (when set) freed (TrackedGlobal_Free) and +0xc cleared; the count
// cleared. Returns 1.
u32 __fastcall SpriteFrames_Free(u8 *set);

// 0x588d90 (thiscall; width, height, pops 8; name unconfirmed): unless the s16 sizes at +0x34 / +0x36 are already these, the image's
// (+0x28, when set, its array set and count not 0) textures freed; then +0x34 / +0x36 take the low words.
void __fastcall SpriteFrame_SetSize(u8 *frame, void *, s32 width, s32 height);

// 0x588e40 (thiscall; width, height, pops 8; name unconfirmed): the same for the sizes at +0x30 / +0x32, but the height is compared with
// the s16 at +0x30 again (sic) and the image's array is not checked (only the image and the count).
void __fastcall SpriteFrame_SetSize30(u8 *frame, void *, s32 width, s32 height);

// 0x5890e0 (thiscall; quad (four x, y, z, w vertices), width, height: floats; pops 0xc; name unconfirmed): x87 in the original's order.
// A width or height of 0 (or a NaN) is the frame's scale (+0x2c) * 20, the width also times [0x8bcbcc]. With the image (+0x28; u16
// +0xc / +0xe its size, u16 +0x10 / +0x12 the full size, s16 +0x14 / +0x16 the offset): left = offset x / full width * width, right
// = (size x + offset x) / full width * width, top = offset y / full height * height, bottom = (size y + offset y) / full height *
// height. Flag 0x10 (+0x10) negates left and right (1 returned), flag 0x20 or 0x40 top and bottom (the result flipped). Vertices 0
// and 3 get x = left + x0, 1 and 2 right + x1. With flag 0x200 and bit 0x1000000 of 0x8bcbc8 the frame is picked ([0x8bd4fc] =
// frame) when the pointer at 0x8bd4f0 / 0x8bd4f4 is within sqrt(((right - left)^2 + (bottom - top)^2) * 0.0002) of vertex 0 seen
// through 1 / w0, unless the frame picked before has a w (+0xc) at or below w0. Flag 0x80000 (the result flipped): vertices 0 and
// 1 get z = top + z0, 2 and 3 bottom + z2, every y raised by [0x8bd4ec], and flag 0x80 turns the quad in x and z by +0x30
// (Quad_TurnXZ). Otherwise 0 and 1 get y = top + y0, 2 and 3 bottom + y2, flag 0x80 turns it in x and y by +0x30 (Quad_TurnXY about
// its middle) and flag 0x1000 by [0x87af60] about vertex 0 as it came. Returns 0 or 1.
u32 __fastcall SpriteFrame_PlaceQuad(u8 *frame, void *, float *quad, float width, float height);

// 0x589b50 (thiscall (item), pops 4; returns 1; name unconfirmed): a draw item filled from the frame, unless the frame lacks flag
// 0x10000000 (+0x10). The texture (*+0x28; none: +0x10 = 0, +0xc = 0x1256, +0x18-+0x24 = 0): +0x10 its +0x1c with its flag 0x1000
// (+8) else 0, +0xc Texture_FormatBits, +0x18 / +0x1c its +0x28 / +0x2c (x87), +0x20 / +0x24 those plus its +0x20 / +0x24, +0 |= 4 for
// its flag 0x100000; then +0 = 0 in any case (sic). Flag 0x400: +0x18 of the frame (y) raised by the ground under x, z (+0x14, +0x1c:
// the procedural terrain [0x879d60]'s heights (its grid +0x28), cell __ftol((x - grid +8) * +0x10), __ftol((z - grid +0xc) * +0x14),
// the next cell clamped to 0..+8 / +0xc, the cell's own not) plus [0x8bd4ec]. The item's quad (+4, read again each time) then: flag
// 0x40000: x, y, z from +0x14 (y negated, through the x87), w 1, z less 0.00125 when above it, +0x28 = 2, the vertex copied to the other
// three, SpriteFrame_PlaceQuad with (scale +0x2c * 0.05) * [0x8bcbcc] and scale * 0.05. Else +0x28 = 0 and flag 0x80000: x, y, z from
// +0x14 (w as it is), copied on, PlaceQuad with 0, 0, the transformer ([0x87b1d8]) slot 1 (cdecl: it, [0x87b088], the quad twice,
// 0x10, 0x10, 4, 0), +0xc |= 0x800; otherwise the vertex +0 copied in and on, PlaceQuad with 0, 0, and with flag 0x200 and no 0x2000 in
// 0x8bcbc8 the last z (+0x38) less 0.2 / (it + 1), copied to the other three. +8 = the frame's +0x24; +0 |= 2 for PlaceQuad's 1; flag
// 0x8000 swaps +0x1c and +0x24 (the first through the x87).
u32 __fastcall SpriteFrame_FillItem(u8 *frame, void *, u8 *item);

// 0x589410 (thiscall (quad, matrix, width, height), pops 16; name unconfirmed): the frame's image laid on the ground: S = scale +0x2c *
// 20 (x87; as a float from the third use on), left / right = offset x (+ size x) / full width * S * [0x8bcbcc] * width (the image's
// fields as SpriteFrame_PlaceQuad reads them), top / bottom = offset y (+ size y) / full height * S * height * 1; flag 0x10 negates
// left and right, 0x20 top and bottom. With the axis entry at 0x8c1798 + 12 * [0x8c181c] (its x and z): vertices 0 and 3 x = ax * left
// + x, z = left * az + z (the frame's +0x14, +0x1c), 1 and 2 with right; y bottom + y (+0x18) for 0 and 1, top + y for 2 and 3; then
// every y less the least of them (vertex 0's, 1's, 3's, 2's, NaNs as the compares fall). Flag 0x80 turns it (Quad_TurnXY by -+0x30);
// the transformer ([0x87b1d8]) slot 1 (cdecl: it, matrix, the quad twice, 0x10, 0x10, 4, 0); then vertices 3, 2, 1, 0 raised by the
// ground under their x, z (as SpriteFrame_FillItem's) plus +0x20 (flag 0x800) or 0, plus [0x8bd4ec]; the transformer's slot 1 again
// with [0x87b088] for the matrix. Returns the dword at 0x8c1808 + 4 * [0x8c181c] (read again).
u32 __fastcall SpriteFrame_GroundQuad(u8 *frame, void *, float *quad, u32 matrix, float width, float height);
}
