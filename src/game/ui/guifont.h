// The GUI library's font (the vtable 0x7058d8, set up by 0x657d30: +4 0, the spacing words +6 / +8 2, the glyphs +0xc 0: 8 bytes a
// character, the first a signed width; what else they hold is unconfirmed). Reimplementation of 0x658243 and 0x658334. The names are
// unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x658243 (thiscall (count: a word, width: a byte, spacing: a word, height: a word, b: a byte), pops 20; name unconfirmed): the glyphs
// +0xc a new block of count (signed) * 8 bytes (Mem_Alloc); when got: the count +4, each glyph's bytes +0 / +1 the width and b and its
// dword +4 0, the word +0xa the height. The spacing +6 set either way. Returns the font.
u8 *__fastcall GuiFont_Init(u8 *font, void *, u32 count, u32 width, u32 spacing, u32 height, u32 b);

// 0x658334 (thiscall (text: wide, from, to: words), pops 12; name unconfirmed): the width (AX) of the text's characters from `from` to
// `to` (both included): each glyph's width (signed) plus the spacing +6, summed in a word. `to` below `from` (signed) means the text's
// last character (its length - 1). 0 with no text, no glyphs, an empty text, `from` negative or past the length, `to` not past `from`
// (sic: a single character is 0 wide) or past the length; `to` at the length counts the terminator's glyph (sic).
u16 __fastcall GuiFont_TextWidth(const u8 *font, void *, const u16 *text, s16 from, s16 to);
}
