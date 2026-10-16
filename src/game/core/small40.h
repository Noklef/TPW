// Text measuring over wide characters, with the glyph width taken from slot 3 (offset 0xc) of the object: reimplementation of 0x47f9a0 and 0x47fa00. Named by what they do
// until the class is known.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x47f9a0 (thiscall; text, start, end (16-bit words in 32-bit slots), pops 12): the sum of the widths (slot 3, called with the character; the high half of what it is given is
// whatever was left of the last result, sic) of the characters from start up to end (not past a NUL); returns the low half of the sum with the high half of the last
// sign-extended result (or of start).
u32 __fastcall TextMetrics_Width(u8 *object, void *, const u16 *text, u32 start, u32 end);

// 0x47fa00 (thiscall; text, start, width, pops 12): the index of the character at which the width runs out: the remaining width (the low half is tested, sic) loses each
// character's width (slot 3) until it is not above 0 or the text ends; returns the index of the last character taken (start - 1 for none).
s32 __fastcall TextMetrics_FitIndex(u8 *object, void *, const u16 *text, u32 start, u32 width);

}  // extern "C"
