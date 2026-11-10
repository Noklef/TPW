// The GUI library's font (the vtable 0x7058d8, set up by 0x657d30: +4 0, the spacing words +6 / +8 2, the glyphs +0xc 0: 8 bytes a
// character, the first a signed width; what else they hold is unconfirmed). Reimplementation of 0x657da0, 0x657e34, 0x657ee0, 0x658243, 0x6582ec, 0x658334 and 0x6583fe. The
// names are unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x658243 (thiscall (count: a word, width: a byte, spacing: a word, height: a word, b: a byte), pops 20; name unconfirmed): the glyphs
// +0xc a new block of count (signed) * 8 bytes (Mem_Alloc); when got: the count +4, each glyph's bytes +0 / +1 the width and b and its
// dword +4 0, the word +0xa the height. The spacing +6 set either way. Returns the font.
u8 *__fastcall GuiFont_Init(u8 *font, void *, u32 count, u32 width, u32 spacing, u32 height, u32 b);

// 0x6582ec (thiscall (index: a word, width, b: bytes), pops 12; no direct callers; name unconfirmed): with glyphs +0xc and the index
// (signed) below the count +4 (unsigned), the glyph's bytes +0 / +1 = width and b (a negative index writes before the glyphs, sic).
void __fastcall GuiFont_SetGlyph(u8 *font, void *, u32 index, u32 width, u32 b);

// 0x658334 (thiscall (text: wide, from, to: words), pops 12; name unconfirmed): the width (AX) of the text's characters from `from` to
// `to` (both included): each glyph's width (signed) plus the spacing +6, summed in a word. `to` below `from` (signed) means the text's
// last character (its length - 1). 0 with no text, no glyphs, an empty text, `from` negative or past the length, `to` not past `from`
// (sic: a single character is 0 wide) or past the length; `to` at the length counts the terminator's glyph (sic).
u16 __fastcall GuiFont_TextWidth(const u8 *font, void *, const u16 *text, s16 from, s16 to);

// 0x6583fe (thiscall (character: a word), pops 4; name unconfirmed): the character's glyph width (signed) plus the spacing +6, in AX; 0
// with no glyphs or a character not below the count +4.
u16 __fastcall GuiFont_CharWidth(const u8 *font, void *, u32 character);

// 0x657da0 (thiscall (across, down: words), pops 8; no callers; name unconfirmed): the spacing words +6 / +8.
void __fastcall GuiFont_SetSpacing(u8 *font, void *, u32 across, u32 down);

// 0x657e34 (thiscall (x, y: words, text: wide, unused), pops 16; no callers; name unconfirmed): with glyphs +0xc, each character below
// the count +4 drawn through the folded stub 0x416e10 (Save_FieldName (x, y, character): 0, so the pen goes to 0, 0). Returns the pen,
// x in the high word.
u32 __fastcall GuiFont_DrawText(const u8 *font, void *, u32 x, u32 y, const u16 *text, u32 unused);

// 0x657ee0 (thiscall (x, y: words, text, clip, flag), pops 20; no callers; name unconfirmed): DrawText with the clip's slot 0
// (thiscall (x, y), AX nonzero: inside) testing each glyph's corners. Unflagged: each character inside is drawn, the others only
// advance the pen. Flagged (no text: the pen): the clip's top row tested at y and y + the height +0xa, then the characters advanced
// over up to the first one inside, where the loop stops, so the drawing loop after it never runs (sic) and the pen's x is that
// character's (none inside: an unset stack slot, sic).
u32 __fastcall GuiFont_DrawTextClipped(const u8 *font, void *, u32 x, u32 y, const u16 *text, u8 *clip, u32 flag);

// 0x657d6c (thiscall; name unconfirmed): the vtable set back to 0x7058d8, the glyphs +0xc freed when there are any (+0xc kept).
void __fastcall GuiFont_Dtor(u8 *font);

// 0x657dff (thiscall; name unconfirmed): the glyphs +0xc freed when there are any, +0xc cleared.
void __fastcall GuiFont_FreeGlyphs(u8 *font);

// 0x658460 (thiscall (flags), pops 4; name unconfirmed): GuiFont_Dtor, then the font freed with bit 0 of the flags. Returns the font.
u8 *__fastcall GuiFont_DeletingDtor(u8 *font, void *, u32 flags);
}
