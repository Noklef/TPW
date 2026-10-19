// Colours to pixels (0x5fccf0-0x5fd639, thiscall unless noted): an 0xAARRGGBB colour put into a
// surface's pixel format, by hand for the formats the game knows (each a static PixelFormat set
// up before main) and otherwise by rotating each channel's field into place.
//
// The originals' quirks are kept: the ABGR and BGR formats OR the red and blue bytes into each
// other's place instead of swapping them; the 24-bit and XRGB/XBGR results drop the alpha byte,
// the others keep whatever bits fall in; a format matches when the masks and bit count agree and
// both or neither are paletted. The generic path rotates each channel so its top bit lands on the
// field's, keeping as many bits as the field is wide, so a field wider than 8 bits takes in the
// top of the channel below it; it also leaves its working in the colour. The ARGB8888 format at
// 0xfa7d40 is set up but never read.
#pragma once

#include "../../../re/types.h"

struct PixelFormat {
    u32 red, green, blue, alpha;  // masks
    u32 bitCount;                 // +0x10
    u32 paletted;                 // +0x14
};
static_assert(sizeof(PixelFormat) == 0x18, "PixelFormat is 0x18 bytes");

// Each channel's rotation from its 0xAARRGGBB field's top bit to the format's, and the format's
// mask rotated back into 0xAARRGGBB.
struct ColorShifts {
    u32 rotate[4];  // red, green, blue, alpha
    u32 mask[4];    // +0x10
};
struct Color {
    u32 argb;
    ColorShifts shifts;  // +4: what the last generic conversion worked out
};
static_assert(sizeof(Color) == 0x24, "Color is 0x24 bytes");

// The static formats, in the order of their initializers.
enum {
    kFormatArgb8888Unused,  // 0xfa7d40 (0x5fccf0)
    kFormatArgb1555,        // 0xfa7de8 (0x5fcd40)
    kFormatRgb555,          // 0xfa7e18 (0x5fcd90)
    kFormatRgb565,          // 0xfa7d88 (0x5fcde0)
    kFormatArgb4444,        // 0xfa7d70 (0x5fce30)
    kFormatRgb888,          // 0xfa7e00 (0x5fce80)
    kFormatBgr888,          // 0xfa7dd0 (0x5fced0)
    kFormatArgb8888,        // 0xfa7db8 (0x5fcf20)
    kFormatAbgr8888,        // 0xfa7d58 (0x5fcf70)
    kFormatXrgb8888,        // 0xfa7da0 (0x5fcfc0)
    kFormatXbgr8888,        // 0xfa7d28 (0x5fd010)
    kFormatCount
};
struct ColorEnv {
    PixelFormat *format[kFormatCount];
};
// nullptr restores the game's.
void Color_SetEnv(const ColorEnv *e);

extern "C" {
// The static initializers (cdecl), each reached through a jump from the start-up table.
void __cdecl PixelFormat_InitArgb8888Unused();  // 0x5fccf0
void __cdecl PixelFormat_InitArgb1555();        // 0x5fcd40
void __cdecl PixelFormat_InitRgb555();          // 0x5fcd90
void __cdecl PixelFormat_InitRgb565();          // 0x5fcde0
void __cdecl PixelFormat_InitArgb4444();        // 0x5fce30
void __cdecl PixelFormat_InitRgb888();          // 0x5fce80
void __cdecl PixelFormat_InitBgr888();          // 0x5fced0
void __cdecl PixelFormat_InitArgb8888();        // 0x5fcf20
void __cdecl PixelFormat_InitAbgr8888();        // 0x5fcf70
void __cdecl PixelFormat_InitXrgb8888();        // 0x5fcfc0
void __cdecl PixelFormat_InitXbgr8888();        // 0x5fd010
// 0x5fd050 (&format): the colour as a pixel of that format.
u32 __fastcall Color_ToPixel(Color *c, void *edx, const PixelFormat *f);
u32 __fastcall PixelFormat_Equal(const PixelFormat *f, void *edx, const PixelFormat *other);  // 0x5fd400
// 0x5fd460 (red, green, blue, alpha top bits in 0xAARRGGBB, &format).
void __fastcall ColorShifts_Compute(ColorShifts *s, void *edx, u32 red, u32 green, u32 blue, u32 alpha,
                                    const PixelFormat *f);
u32 __fastcall Color_SwapRedBlue(const Color *c);  // 0x5fd5a0: ORed, not swapped
// 0x5fd5c0 (&shifts): each channel's masked bits rotated into place.
u32 __fastcall Color_Shift(const Color *c, void *edx, const ColorShifts *s);

// 0x5f09a0 (cdecl; out, position, a, b, c; the records are nine dwords): position limited to 0 to 100; below 50 the result is a moved towards b by position / 50, from 50 it is b
// moved towards c by (position - 50) / 50. The result is the first record with its bytes 0 to 2 moved (each by the difference of the bytes times the fraction in 16.16 fixed
// point, arithmetic shift, added to the first byte) and its byte 3 set to 0xff. Returns out.
u32 *__cdecl Colour_Blend3(u32 *out, s32 position, const u32 *a, const u32 *b, const u32 *c);

// 0x591270 (cdecl; the table, then a DDPIXELFORMAT passed by value: size, flags, FourCC, bit count, red, green, blue and alpha masks; name unconfirmed): fills 256 records of four dwords
// (alpha, red, green, blue): for each level i and mask, i lowered to the mask's width (the shift is 8 - the count of its set bits, taken modulo 32) and moved up to the mask's lowest set bit
// (0 for an empty mask).
void __cdecl Format_BuildChannelTable(u32 *table, u32 size, u32 flags, u32 fourcc, u32 bits, u32 red, u32 green, u32 blue, u32 alpha);

// 0x575160 (cdecl; name unconfirmed; the texture load's, 0x5750f5): width * height source pixels of 4 bytes (blue, green, red, alpha) or, for
// any other byte count, 3 (no alpha: the record of 0xff) put through such a table (the records' alpha, red, green and blue ORed) and stored
// as dwords at dst, step bytes apart; with flag 1 the rows are taken from the last up (each left to right). Each pixel's bytes are summed
// as read again after its store (in place they are the converted ones); the 3-byte forms count 0xff per pixel for alpha, the flipped one
// only for rows of a positive width. The averages (each sum / width (signed), times height, sic) packed alpha, red, green, blue into
// [0x87a25c] (a width of 0 faults). Returns 1 for 4 bytes (else 0), with 2 when the pixels whose alpha is 1-0xef number at least
// count * 0.05 (x87, truncated).
u32 __cdecl Format_ConvertPixels(u8 *dst, const u8 *src, s32 width, s32 height, const u32 *table, s32 step, s32 bytes, u32 flags);
}
