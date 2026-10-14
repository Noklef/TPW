// The software drawing primitives: the Bullfrog library's "lbDisplay" state (0xfa76b8) and one
// table of 16 thiscall primitives per pixel depth (0x7034d8 8 bits, 0x703518 16, 0x703558 24,
// 0x703598 32). A DrawDriver is one dword pointing at a table; the screen code calls through
// the current one (0xfa76d0), and the primitives call each other the same way.
//
// Slots: 0 pixel (x, y), 1 box (rect: 2 outline or 3 filled), 4 circle (x, y, r: 5 outline or
// 6 filled), 7 line (x1, y1, x2, y2), 8 horizontal and 9 vertical line (x, y, length),
// 10 triangle, 11 clear the clip rectangle, 12/13 debug text (narrow/wide), 14/15 sprites.
// Each ends with a DrawColour, passed by value: the paletted primitives draw its index.
//
// Flags (+0x3c): 4 outlines instead of filling, 8 and 0x10 blend through the 256x256 ghost
// table (colour*256 + background, or background*256 + colour), 0x20 remaps the colour through
// the remap table first (plain drawing only).
//
// The originals' quirks are kept: the line primitives accept x == right (clipped to length 0);
// the debug text's tab stop is (x + 32) & 0xffe0, which drops x's high bits; clearing an empty
// clip rectangle with a negative width writes ~4 GB, as the original would.
#pragma once

#include "../../../re/types.h"
#include "../math/rect.h"
#include "surface.h"

struct DrawColour {
    u32 rgb;
    u8 index;   // +4: the palette index
    u8 pad[3];  // whatever was on the caller's stack
};
static_assert(sizeof(DrawColour) == 8, "DrawColour is 8 bytes");

struct DrawDriver {
    void *const *table;
};

struct DrawContext {           // 0xfa76b8
    const u8 *remap;         // +0: colour remap for flag 0x20
    u32 unknown04;
    Rect clip;               // +8 (0xfa76c0)
    DrawDriver *driver;      // +0x18 (0xfa76d0): the current depth's
    u32 unknown1c;
    s32 unknown20[4];        // +0x20 (0xfa76d8)
    SurfaceInfo *surface;    // +0x30 (0xfa76e8): pitch at +0x24
    u32 unknown34;
    DrawDriver driver8;      // +0x38 (0xfa76f0)
    u32 flags;               // +0x3c (0xfa76f4)
    DrawDriver driver16;     // +0x40
    DrawDriver driver24;     // +0x44
    DrawDriver driver32;     // +0x48
    u8 *bits;                // +0x4c (0xfa7704): the surface's pixels
    PixelShifts shifts;      // +0x50 (0xfa7708)
    const u8 *ghost;         // +0x70 (0xfa7728): 256x256 blend table
};
static_assert(sizeof(DrawContext) == 0x74, "DrawContext is 0x74 bytes");

enum : u32 {
    kDrawOutline = 4,
    kDrawGhost = 8,         // ghost[colour * 256 + background]
    kDrawGhostReverse = 0x10,  // ghost[background * 256 + colour]
    kDrawRemap = 0x20,
};

extern "C" {
// Shared by all depths.
void __fastcall Draw_Box(DrawDriver *d, void *edx, const Rect *r, DrawColour c);         // 0x61bc90 (1)
void __fastcall Draw_BoxOutline(DrawDriver *d, void *edx, const Rect *r, DrawColour c);  // 0x628aa0 (2)
void __fastcall Draw_Circle(DrawDriver *d, void *edx, s32 x, s32 y, s32 r, DrawColour c);  // 0x629bf0 (4)
void __fastcall Draw_Triangle(DrawDriver *d, void *edx, s32 x1, s32 y1, s32 x2, s32 y2, s32 x3, s32 y3,
                              DrawColour c);  // 0x61eab0 (10)
// 8 bits.
void __fastcall Draw8_Pixel(DrawDriver *d, void *edx, s32 x, s32 y, DrawColour c);         // 0x61baf0 (0)
void __fastcall Draw8_BoxFill(DrawDriver *d, void *edx, const Rect *r, DrawColour c);      // 0x61eed0 (3)
void __fastcall Draw8_Line(DrawDriver *d, void *edx, s32 x1, s32 y1, s32 x2, s32 y2, DrawColour c);  // 0x61bce0 (7)
void __fastcall Draw8_HLine(DrawDriver *d, void *edx, s32 x, s32 y, s32 length, DrawColour c);  // 0x61c2e0 (8)
void __fastcall Draw8_VLine(DrawDriver *d, void *edx, s32 x, s32 y, s32 length, DrawColour c);  // 0x61c490 (9)
void __fastcall Draw8_Clear(DrawDriver *d, void *edx, DrawColour c);                       // 0x61ee30 (11)
void __fastcall Draw8_Text(DrawDriver *d, void *edx, s32 x, s32 y, const char *s, DrawColour c);   // 0x61f0a0 (12)
void __fastcall Draw8_TextW(DrawDriver *d, void *edx, s32 x, s32 y, const u16 *s, DrawColour c);  // 0x61f220 (13)
}

struct DrawEnv {
    DrawContext *state;  // 0xfa76b8
    const u8 *font;    // 0x778108: 6 rows of 5 pixels (bits 0x40-0x04) and the advance, 8 bytes a glyph
};
// The state and tables go through env (nullptr: the game's).
void Draw_SetEnv(const DrawEnv *env);
