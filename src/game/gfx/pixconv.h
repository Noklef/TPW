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
}

struct PixConvEnv {
    void(__cdecl *free)(void *block);  // Mem_Free (0x67acb0)
};
// The calls go through env (nullptr: the game's).
void PixConv_SetEnv(const PixConvEnv *env);
