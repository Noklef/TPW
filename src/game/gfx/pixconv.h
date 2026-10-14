// The pixel converters (vtables 0x7039c0 base, 0x7039a0 palette, 0x7039b0): objects that turn
// a row of source pixels into a surface's format; slot +4 converts forwards, +8 backwards from
// the row's end (so a row can widen in place). The palette converter (0x639230-0x6394c0) maps
// 8-bit indices through a 256-entry palette (0x00BBGGRR, as PALETTEENTRY reads) into 1-4 byte
// pixels: each channel is masked, shifted down to drop its low bits and up into the target's
// field, the alpha mask ORed in, and the low bpp bytes written.
//
// The originals' quirks are kept: Mask_Shifts keeps the top min(source, target) bits of a
// channel and puts them at the target field's lowest bit, so a target field wider than the
// source is filled from the bottom; a channel the target lacks (mask 0) shifts the source out
// entirely (or by 31 when that would be 32 or more); only the source's top run of bits counts;
// rows of other byte widths (0, 5 and up) are left alone.
//
// The RGB converter (0x6396b0-0x63a7a0) moves direct-colour pixels between two formats: each
// channel is masked and rotated (not shifted) so the source field's top bit lands on the
// target's, then masked to the target field; when any target field is wider than its source,
// every channel also ORs in a second rotation by its source width, repeating the top bits once
// into the low ones. A target alpha with no source alpha is filled solid. Quirks kept: source
// pixels are read as whole dwords whatever their size (up to 3 bytes past the row); bits that
// rotate round land in the target's low bits; a channel either side lacks gets rotation 0 and
// keeps its old width (Init leaves it unset).
//
// The index converter (0x63a7b0-0x63ad80, vtable 0x703fa8) goes the other way, direct colour to
// a 256-entry palette: with its 32K table (5 bits a channel, built once per palette pointer by
// the inverse-colormap builder 0x642150/0x642190) a pixel is one lookup; without it (the
// allocation failed) each channel is cut to 8 bits (shifted as a byte) and the nearest palette
// entry searched for, the first of equals winning. Quirks kept: a palette is only rebuilt when
// its pointer changes, so new colours at the same address keep the old table; the table is not
// cleared on free; a failed work-buffer allocation still builds into null.
//
// The 16-bit converter (0x63ae10-0x63b2b5, vtable 0x703fb8) packs two RGB888 (3 bytes) or
// ARGB8888 pixels into a dword of two 16-bit ones: each source byte keeps its top bits (as many
// as the target field is wide) and is rotated into place in both halves at once; RGB888 sources
// get the target's alpha mask ORed in. Quirks kept: Init does nothing unless the source equals
// one of the game's two formats (0xfaa268, 0xfaa280; the paletted flags need only agree on zero)
// and the target is 16 bits; a field's rotation counts the gaps in a broken mask; the count is
// halved (toward zero), so an odd last pixel is dropped and a count of 0 or 1 runs the loop
// 2^32 times.
//
// A conversion (0x630880-0x630c4d) owns one converter and picks its class from the two formats
// (Setup): RGB888/ARGB8888 into 16 bits (unless flags bit 0) takes the 16-bit converter, other
// direct colour the RGB one, a paletted source the palette one, a paletted target the index one
// (both need the palette; paletted to paletted gets nothing); the same class again is only
// re-initialised. A row runs backwards when the pixels widen, so it can convert in place.
// Quirks kept: Setup re-initialises a null converter (after a failed allocation) when the class
// matches; Rect, converting in place, runs backwards when the pixels narrow (the wrong way round,
// unlike Row); the destructor leaves the class alone when there is no converter.
#pragma once

#include "../../../re/types.h"

struct PaletteConverter {    // vtable 0x7039a0, 0x430 bytes
    void *const *vtable;
    u32 mask[3];             // +4: red, green, blue of the palette entries (0xff, 0xff00, 0xff0000)
    u32 right[3];            // +0x10
    u32 left[3];             // +0x1c
    u32 alpha;               // +0x28: ORed into every pixel
    u32 bytes;               // +0x2c: per pixel
    u32 palette[256];        // +0x30
};
static_assert(sizeof(PaletteConverter) == 0x430, "PaletteConverter is 0x430 bytes");

struct RgbConverter {        // vtable 0x7039b0, 0x54 bytes
    void *const *vtable;
    u32 rotate[4];           // +4: red, green, blue, alpha
    u32 sourceMask[4];       // +0x14
    u32 targetMask[4];       // +0x24
    s32 width[4];            // +0x34: the source fields' widths
    u32 targetBytes;         // +0x44: per pixel
    u32 sourceBytes;         // +0x48
    u32 fill;                // +0x4c: ORed into every pixel
    u32 widens;              // +0x50
};
static_assert(sizeof(RgbConverter) == 0x54, "RgbConverter is 0x54 bytes");

struct IndexConverter {      // vtable 0x703fa8, 0x43c bytes
    void *const *vtable;
    u32 palette[256];        // +4
    const u32 *built;        // +0x404: the palette the table was built from
    u8 *table;               // +0x408: 1 << 15 indices
    u32 bits;                // +0x40c: per channel in the table (5)
    u32 levels;              // +0x410: 1 << bits
    u32 mask[3];             // +0x414: the source's red, green, blue
    u32 right[3];            // +0x420
    u32 left[3];             // +0x42c
    u32 bytes;               // +0x438: per source pixel
};
static_assert(sizeof(IndexConverter) == 0x43c, "IndexConverter is 0x43c bytes");

struct Rgb16Converter {      // vtable 0x703fb8, 0x2c bytes
    void *const *vtable;
    u32 from24;              // +4: the source is RGB888 (else ARGB8888)
    u32 fill;                // +8: ORed into every pair (RGB888 sources)
    u32 rotate[4];           // +0xc: red, green, blue, alpha
    u32 keep[4];             // +0x1c: the top bits of a source byte kept, in both halves
};
static_assert(sizeof(Rgb16Converter) == 0x2c, "Rgb16Converter is 0x2c bytes");

struct PixelConversion {
    void *converter;         // vtable +0 deleting destructor, +4 forwards, +8 backwards
    u32 sourceBytes;         // +4: per pixel
    u32 targetBytes;         // +8
    u32 kind;                // +0xc: 0 16-bit, 1 RGB, 2 palette, 3 index, 4 none
};
static_assert(sizeof(PixelConversion) == 0x10, "PixelConversion is 0x10 bytes");

extern "C" {
// 0x630c50 (flags): the base's deleting destructor. 0x630c90: the destructor (the base's
// vtable). 0x630c70 (flags): the palette (and 0x7039b0) converter's deleting destructor.
void *__fastcall PixelConverter_DeletingDtor(void *c, void *edx, u32 flags);
void __fastcall PixelConverter_Dtor(void *c);
void *__fastcall PixelConverter_DeletingDtor2(void *c, void *edx, u32 flags);
// 0x639230 (palette, &format: red, green, blue, alpha masks, bit count).
void __fastcall PaletteConverter_Init(PaletteConverter *c, void *edx, const u32 *palette, const u32 *format);
// 0x6392c0 / 0x6394d0 (dst, src, count; +4 / +8): count indices into pixels.
void __fastcall PaletteConverter_Convert(PaletteConverter *c, void *edx, u8 *dst, const u8 *src, u32 count);
void __fastcall PaletteConverter_ConvertBack(PaletteConverter *c, void *edx, u8 *dst, const u8 *src, u32 count);
// 0x63acb0 (source mask, target mask, &right, &left): the shifts that move a channel.
void __stdcall Mask_Shifts(u32 source, u32 target, u32 *right, u32 *left);
// 0x6396b0 (&source, &target format: red, green, blue, alpha masks, bit count).
void __fastcall RgbConverter_Init(RgbConverter *c, void *edx, const u32 *source, const u32 *target);
// 0x639780 / 0x639f40 (dst, src, count; +4 / +8).
void __fastcall RgbConverter_Convert(RgbConverter *c, void *edx, u8 *dst, const u8 *src, u32 count);
void __fastcall RgbConverter_ConvertBack(RgbConverter *c, void *edx, u8 *dst, const u8 *src, u32 count);
// 0x63a700 (source mask, target mask, &rotate, &width): sets widens when the target is wider.
void __fastcall RgbConverter_ChannelShift(RgbConverter *c, void *edx, u32 source, u32 target, u32 *rotate,
                                          s32 *width);
// 0x63a7b0 (&source format, palette): returns c. 0x63a810 (flags) / 0x63a830: the destructors.
IndexConverter *__fastcall IndexConverter_Ctor(IndexConverter *c, void *edx, const u32 *format, const u32 *palette);
void *__fastcall IndexConverter_DeletingDtor(IndexConverter *c, void *edx, u32 flags);
void __fastcall IndexConverter_Dtor(IndexConverter *c);
// 0x63a880 (&source format, palette).
void __fastcall IndexConverter_Init(IndexConverter *c, void *edx, const u32 *format, const u32 *palette);
// 0x63aa20: frees the table (and leaves the pointer).
void __fastcall IndexConverter_FreeTable(IndexConverter *c);
// 0x63aa40 / 0x63ab70 (dst, src, count; +4 / +8).
void __fastcall IndexConverter_Convert(IndexConverter *c, void *edx, u8 *dst, const u8 *src, u32 count);
void __fastcall IndexConverter_ConvertBack(IndexConverter *c, void *edx, u8 *dst, const u8 *src, u32 count);
// 0x63ad80 (red, green, blue; low bytes): the nearest palette entry.
u8 __fastcall IndexConverter_Nearest(IndexConverter *c, void *edx, u32 red, u32 green, u32 blue);
// 0x63ae10 (&source, &target format): returns c. 0x63ae90: Init.
Rgb16Converter *__fastcall Rgb16Converter_Ctor(Rgb16Converter *c, void *edx, const u32 *source, const u32 *target);
void __fastcall Rgb16Converter_Init(Rgb16Converter *c, void *edx, const u32 *source, const u32 *target);
// 0x63ae60 (flags) / 0x63ae80: the destructors (the base's vtable).
void *__fastcall Rgb16Converter_DeletingDtor(void *c, void *edx, u32 flags);
void __fastcall Rgb16Converter_Dtor(void *c);
// 0x63b140 (dst, src, count; +4): count pixels, two at a time.
void __fastcall Rgb16Converter_Convert(Rgb16Converter *c, void *edx, u8 *dst, const u8 *src, s32 count);
// 0x630880: no converter. 0x630c30: deletes it.
PixelConversion *__fastcall PixelConversion_Ctor(PixelConversion *c);
void __fastcall PixelConversion_Dtor(PixelConversion *c);
// 0x630890 (&target, &source format, palette, flags).
void __fastcall PixelConversion_Setup(PixelConversion *c, void *edx, const u32 *target, const u32 *source,
                                      const u32 *palette, u32 flags);
// 0x630b70 (dst, src, count).
void __fastcall PixelConversion_Row(PixelConversion *c, void *edx, u8 *dst, const u8 *src, u32 count);
// 0x630ba0 (dst or null for in place, src, dst pitch, src pitch, rows, width).
void __fastcall PixelConversion_Rect(PixelConversion *c, void *edx, u8 *dst, const u8 *src, s32 dstPitch,
                                     s32 srcPitch, u32 rows, u32 width);
}

struct PixConvEnv {
    void(__cdecl *free)(void *block);  // Mem_Free (0x67acb0)
    void *(__stdcall *alloc)(u32 size);  // 0x642860, counted
    s32(__stdcall *release)(void *block);  // 0x642880, counted
    void *(__fastcall *mapInit)(void *map, void *edx);  // 0x642150: the 0x58-byte builder
    void(__fastcall *mapBuild)(void *map, void *edx, u32 first, u32 count, const u32 *palette, u32 bits,
                               u32 *work, u8 *table);  // 0x642190
    void(__fastcall *mapDone)(void *map, void *edx);  // 0x642180
    const u32 *rgb888;    // 0xfaa268: masks, bit count, paletted
    const u32 *argb8888;  // 0xfaa280
    void *(__cdecl *allocate)(u32 size);  // Mem_Alloc (0x67acc0)
};
// The calls go through env (nullptr: the game's).
void PixConv_SetEnv(const PixConvEnv *env);
