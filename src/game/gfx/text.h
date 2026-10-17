// Text drawn through a pluggable text driver (0x5f8db0-0x5f8f70): the current driver (0xfa76a0)
// and its context (0xfa76a4) are set by Text_SetDriver; Text_Draw walks a string through the
// driver's glyph callback. The game's two drivers (0x7031b0 plain, 0x7031c0 in one colour) have
// no init or shutdown; their context is a font, RleSprites from ' ' on, measured by
// Font_Measure and drawn as sprites through the current draw driver's slots 14 and 15
// (Draw_Sprite 0x61b030, Draw_SpriteShape 0x61b050).
//
// The originals' quirks are kept:
// - A newline advances y by the height of a space (Text_LineHeight), x back to where it began.
// - A glyph is font[ch - ' '] with no bounds: characters below ' ' read before the font.
// - Text_SetDriver installs the new driver before its init runs and drops it (to none) when the
//   init fails; without an init the parameter becomes the context.
#pragma once

#include "../../../re/types.h"
#include "draw.h"

struct TextSize {
    u32 width;
    u32 height;
};

struct TextDriver {  // 0x7031b0, 0x7031c0
    s32 *(__cdecl *init)(s32 *result, void **context, u32 param);
    void(__cdecl *shutdown)(void *context, const TextDriver *next, u32 param);  // +4
    u32(__cdecl *measure)(void *context, TextSize *size, u32 ch);  // +8
    u32(__cdecl *draw)(void *context, s32 x, s32 y, u32 ch, DrawColour colour);  // +0xc: the advance
};

struct TextEnv {
    const TextDriver **driver;  // 0xfa76a0
    void **context;             // 0xfa76a4
    DrawDriver **drawDriver;    // 0xfa76d0: the draw context's current driver
};
// nullptr restores the game's.
void Text_SetEnv(const TextEnv *e);

extern "C" {
// Characters from x, '\n' back to x a line down.
void __cdecl Text_Draw(s32 x, s32 y, const char *text, DrawColour colour);
// (&out, driver, its parameter): 0, or -1 when its init fails.
s32 *__cdecl Text_SetDriver(s32 *out, const TextDriver *driver, u32 param);
// The height of ch, as the driver measures it (its return is ignored).
u32 __cdecl Text_LineHeight(u32 ch);
// The fonts' glyph callbacks: the glyph drawn, its width the advance.
u32 __cdecl Font_DrawGlyph(const RleSprite *font, s32 x, s32 y, u32 ch, DrawColour colour);
u32 __cdecl Font_DrawGlyphShape(const RleSprite *font, s32 x, s32 y, u32 ch, DrawColour colour);
// The glyph's width and height; returns the font (EAX as left, unused).
u32 __cdecl Font_Measure(const RleSprite *font, TextSize *size, u32 ch);
// The current draw driver's slot 14 / 15.
void __cdecl Draw_Sprite(s32 x, s32 y, const RleSprite *sprite);
void __cdecl Draw_SpriteShape(s32 x, s32 y, const RleSprite *sprite, DrawColour colour);

// 0x47f9a0 (thiscall; text, start, end (16-bit words in 32-bit slots), pops 12): the sum of the widths (slot 3, called with the character; the high half of what it is given is
// whatever was left of the last result, sic) of the characters from start up to end (not past a NUL); returns the low half of the sum with the high half of the last
// sign-extended result (or of start).
u32 __fastcall TextMetrics_Width(u8 *object, void *, const u16 *text, u32 start, u32 end);

// 0x47fa00 (thiscall; text, start, width, pops 12): the index of the character at which the width runs out: the remaining width (the low half is tested, sic) loses each
// character's width (slot 3) until it is not above 0 or the text ends; returns the index of the last character taken (start - 1 for none).
s32 __fastcall TextMetrics_FitIndex(u8 *object, void *, const u16 *text, u32 start, u32 width);

// 0x50e260 (stdcall; out, text): the globals 0x7cf100 and 0x7cf104 = 0 and the two dwords at out = those (the original's debug printf is
// where a measurement would be made).
void __stdcall TextExtent_Zero(u32 *out, const u8 *text);

// 0x47f990 (thiscall; name unconfirmed): the object at +0x14, when set, answers with its virtual call 8 (a 16-bit value); 0 (AX only) when it is not set.
u32 __fastcall TextMetrics_ForwardSlot8(u8 *object);
}
