// The text layout engine (0x6ad000-0x6b1000) the GUI's text drawing (0x492750, 0x4929f0) builds on the stack: three nested objects.
// - The text style (vtable 0x70a0d8, 0x50 bytes; the global one at 0x78ba2c): +4 the font (a reference-counted object: slot 0 releases,
//   slot 1 adds a reference, slot 9 answers its flags), +8 the font's flags, +0xc a state (3 after construction or a font change, 1 after a
//   colour), +0x10 1, +0x14 a SurfaceInfo (0x28 bytes), +0x3c, +0x40, +0x44 the colour (5 bytes), +0x4c.
// - The style holder (vtable 0x70a13c, 0x58 bytes): +4 the style, +0x54.
// - The layout (vtable 0x70a10c, 0xbc bytes): +4 the holder; +0x5c, +0x70 and +0x84 one byte each before three vectors (begin, end, capacity
//   at +0x60, +0x74, +0x88), +0x94 a flag, +0x98, +0x9c, +0xa0 / +0xa4 two kinds, +0xac-+0xb8.
// Reimplementation of 0x6ad5d0, 0x6ad620, 0x6ad640, 0x6ad6e0, 0x6ad7c0, 0x6ade00, 0x6ade90, 0x6adeb0, 0x6b0230, 0x6b0250 and 0x6b0270, and
// the layout's text setting, placing, drawing and measuring (0x6ae930, 0x6ae550, 0x6ae0f0, 0x6ae1c0).
// The names are unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x6ad5d0 (thiscall): the font (+4) cleared, the state (+0xc) 3, +0x10 1, the SurfaceInfo at +0x14 constructed (SurfaceInfo_Ctor),
// +0x3c, +0x40, the colour bytes +0x44-+0x48 and +0x4c cleared, the vtable 0x70a0d8. Returns this.
u8 *__fastcall TextStyle_Ctor(u8 *style);

// 0x6ad640 (thiscall): the vtable 0x70a0d8; a font (+4) is released (its slot 0) and cleared.
void __fastcall TextStyle_Dtor(u8 *style);

// 0x6ad620 (thiscall (flags), pops 4): TextStyle_Dtor, then the object freed (Mem_Free) for flags bit 0. Returns this.
u8 *__fastcall TextStyle_DeletingDtor(u8 *style, void *, u32 flags);

// 0x6ad7c0 (thiscall (status, font), pops 8): the state (+0xc) 3; the old font released (slot 0) and cleared; the new one stored, and when
// it is not 0 given a reference (slot 1) and its flags (slot 9) stored at +8. *status = 0. Returns status.
s32 *__fastcall TextStyle_SetFont(u8 *style, void *, s32 *status, u8 *font);

// 0x6ad6e0 (thiscall (status, colour), pops 8): with a font (+4) whose flags (+8) have bit 1, the colour's 5 bytes go to +0x44, the state
// (+0xc) becomes 1 and *status = 0; otherwise *status = -1. Returns status.
s32 *__fastcall TextStyle_SetColour(u8 *style, void *, s32 *status, const u8 *colour);

// 0x6ad660 (thiscall (source), pops 4): unless source is this, +0x10, the colour (+0x44, 5 bytes) and the font (+4) copied, the font given a
// reference (slot 1; no null check and the old font is not released, sic), then +8, +0xc, the SurfaceInfo (+0x14, 0x28 bytes), +0x3c, +0x40
// and +0x4c copied. Returns this.
u8 *__fastcall TextStyle_Assign(u8 *style, void *, const u8 *source);

// 0x6ad820 (thiscall): the font (+4), given a reference (slot 1) when it is not 0.
u8 *__fastcall TextStyle_GetFont(u8 *style);

// 0x6ad840 (thiscall): the font's kind (its slot 10, a tail call), or 5 without a font.
s32 __fastcall TextStyle_FontKind(u8 *style);

// 0x6b0230 (thiscall): the style at +4 constructed, the vtable 0x70a13c, +0x54 cleared. Returns this.
u8 *__fastcall TextStyleHolder_Ctor(u8 *holder);

// 0x6b0270 (thiscall): the vtable 0x70a13c, then the style at +4 destroyed (a jump to TextStyle_Dtor).
void __fastcall TextStyleHolder_Dtor(u8 *holder);

// 0x6b0250 (thiscall (flags), pops 4): TextStyleHolder_Dtor, then the object freed (Mem_Free) for flags bit 0. Returns this.
u8 *__fastcall TextStyleHolder_DeletingDtor(u8 *holder, void *, u32 flags);

// 0x6b0280 (thiscall (status, style), pops 8): +0x54 cleared; a style whose state (+0xc, Source_Field0c) is 3 or more or whose font kind is
// 5 or more (signed) gives *status = -1. Otherwise the style is copied in (TextStyle_Assign) and +0x54 becomes the renderer from the table at
// 0x782d38, indexed ((+0x10 * 5 + font kind) * 3 + state) of the copy (no bounds, sic); *status 0 when it is not 0, else -1. Returns status.
s32 *__fastcall TextStyleHolder_SetStyle(u8 *holder, void *, s32 *status, const u8 *style);

// 0x6b0300 (thiscall (status, a, b, c), pops 0x10): without a renderer (+0x54) *status = -1; otherwise its slot 1 is called with
// (status, a, b, c, the style at +4). Returns status.
s32 *__fastcall TextStyleHolder_Render(u8 *holder, void *, s32 *status, u32 a, u32 b, u32 c);

// 0x6ade00 (thiscall): the holder at +4 constructed; +0x5c, +0x70 and +0x84 get the top byte of `this` (the byte the original reads from its
// pushed ECX, sic: an allocator temporary that was never set); the vectors at +0x60, +0x74, +0x88, the word +0x6c, the byte +0x94 and the
// dwords +0x98, +0x9c, +0xac-+0xb8 cleared (+0xa0-+0xa8 are left); the vtable 0x70a10c. Returns this.
u8 *__fastcall TextLayout_Ctor(u8 *layout);

// 0x6adeb0 (thiscall): the vtable 0x70a10c; the vectors at +0x88, +0x74 and +0x60 (in that order) freed (Mem_Free, null too) and their
// three dwords cleared; then the holder at +4 destroyed.
void __fastcall TextLayout_Dtor(u8 *layout);

// 0x6ade90 (thiscall (flags), pops 4): TextLayout_Dtor, then the object freed (Mem_Free) for flags bit 0. Returns this.
u8 *__fastcall TextLayout_DeletingDtor(u8 *layout, void *, u32 flags);

// 0x6aeab0 (stdcall (character), pops 4; its callers pass the layout in ECX, unused): a character's class from its low word: 0x12 for a tab
// or a space, 4 for a line feed, 0xc for a carriage return, else 1. Only AL is set (the rest of EAX is what it was).
u8 __stdcall TextLayout_CharClass(u32 character);

// 0x6aeda0 (thiscall (first, last), pops 8): erase from a vector of 12-byte elements (begin +4, end +8): the elements from last to the end
// moved down to first (a do-while: last past the end runs on, sic), the end set after them. Returns first.
u8 *__fastcall Vector12_Erase(u8 *vector, void *, u8 *first, u8 *last);

// 0x6aedf0 / 0x6af0a0 (thiscall (position, count, value), pops 0xc): MSVC6's vector<T>::insert(position, count, value) for an 8- / 16-byte
// T (first +4, last +8, end +0xc). With room, the tail moved up and count copies of *value written at position; without, a new block of
// (max(count, size) + size) elements (Mem_Alloc; a negative count clamped at 0 for the allocation but not for the end), the three parts
// copied in (null destinations skipped), the old block freed (Mem_Free). *value is read again for every copy.
void __fastcall Vector8_InsertN(u8 *vector, void *, u8 *position, u32 count, const u8 *value);
void __fastcall Vector16_InsertN(u8 *vector, void *, u8 *position, u32 count, const u8 *value);

// 0x6af3b0 (thiscall (position, count, value), pops 0xc): the same for a 12-byte T (its counts a signed division by 12).
void __fastcall Vector12_InsertN(u8 *vector, void *, u8 *position, u32 count, const u8 *value);

// 0x6add90 (thiscall (first, shown, next, width), pops 0x10) on the layout's lines (+0x70: a 12-byte vector at +4, the widest line at
// +0x10): the line {first, next, width} appended (Vector12_InsertN at the end), +0x10 raised to width (unsigned), and the 16-byte character
// records from shown up to next (a do-while) get flag 0x40 in their byte +4.
void __fastcall TextLines_Append(u8 *lines, void *, u8 *first, u8 *shown, u8 *next, u32 width);

// 0x6aec80 (thiscall (begin, end), pops 8): breaks the layout's 16-byte character records (+4 flags: 2 a space, 4 a line break (no width),
// 0x10 a break allowed; +8 the width) from begin to end into lines no wider than +0xa8 (unsigned), appending each (TextLines_Append on
// +0x70): an overflowing line goes back to the last record with flag 0x10 (its trailing spaces not shown), or breaks where it overflowed;
// a lone record wider than the line takes a line of its own. An empty range is one empty line. Returns the number of lines.
s32 __fastcall TextLayout_BreakLines(u8 *layout, void *, u8 *begin, u8 *end);

// 0x6aeb10 (thiscall (status, left, top, right, bottom), pops 0x14): the lines cleared (Vector12_Erase, the widest +0x80 = 0); without
// records (+0x60 = +0x64) *status = 0 and nothing else. Otherwise each paragraph (up to a record with flag 4, a following record with
// flags 4 and 8 taken along) broken into lines (TextLayout_BreakLines); then the text box placed: +0xb0 / +0xb8 the top and bottom of
// lines * the line height (+0xbc) by the vertical kind (+0xa0: 2 centred, 5 at the bottom, else at the top), +0xac / +0xb4 the left and
// right of the widest line by the horizontal kind (+0xa4: 1 at the right, 2 centred, else at the left). *status = 0. Returns status.
s32 *__fastcall TextLayout_Arrange(u8 *layout, void *, s32 *status, s32 left, s32 top, s32 right, s32 bottom);

// 0x6adfb0 (thiscall (status, rect), pops 8): the positions (an 8-byte vector at +0x84: first +0x88, last +0x8c) resized to one (a {0, 0}
// inserted, Vector8_InsertN, or the rest erased); the origin (+0x98, +0x9c) the rect's left / top, moved by the vertical kind (+0xa0: 2
// centred on the line height +0xbc with an unsigned halving, sic; 5 the bottom less the line height) and the horizontal kind (+0xa4: 1 the
// right, 2 centred), and copied into the first position. *status = 0. Returns status.
s32 *__fastcall TextLayout_SetOrigin(u8 *layout, void *, s32 *status, const s32 *rect);

// 0x6ae930 (thiscall (status, text, style), pops 0xc): the character records (+0x60) emptied and their counter (+0x6c) cleared; the
// style's font (TextStyle_GetFont; its reference never released, sic), none giving *status = -1; the line height (+0xbc) its slot 8's low
// word; its code table (slot 11: +8 the first lead byte + 1, +0xa the glyph count, +0xc the u16 glyphs), none giving *status = -1. Then each
// byte of the text (a lead byte, >= the table's +8 after the -1, takes the next byte along: (c + 0x100) * 255 - lead * 254 + next, which can
// step over the terminator, sic) appends a record (Vector16_InsertN): +0 the glyph (0 past the table), +2 the code, +4 the glyph's class
// (TextLayout_CharClass), +8 the width (the dword the font's slot 7 (out, code) answers a pointer to), +0xc the counter. The code is worked
// in the low word of a dword whose high word is left over (the records' begin at first), and slot 7 gets the whole dword (sic); the
// record's bytes +5-+7 and +0xe-+0xf are never set (the original's stack garbage, sic). *status = 0. Returns status.
s32 *__fastcall TextLayout_SetText(u8 *layout, void *, s32 *status, const u8 *text, u8 *style);

// 0x6ae550 (thiscall (status, style, bounds, render), pops 0x10): the positions (+0x84) emptied and then sized to one per record and one
// more (zeros inserted, Vector8_InsertN); the style taken by the holder (TextStyleHolder_SetStyle, a failure giving *status = -1). The pen
// starts at the text box's top left (+0xac, +0xb0). Each line (+0x74: first, next, width) reordered for right to left reading
// (Records16_ReorderRtl) and started at the box's left, right (+0xa4 1) or centre (2, an unsigned halving, sic); each record's position
// (its +0xc indexes the positions) set to the pen, and (unless the index is below the line's last) the cursor (+0x98, +0x9c) to it, moved
// on for a left to right character (a space before a right to left one is not one) by its width, or for a line break to the box's left /
// right / centre and a line down. A record that is not a line break advances the pen by its width, and with render and no flag 0x40 is
// drawn (TextStyleHolder_Render (status, &pen, code (the high word left over, sic), &glyph rect)), a non-empty glyph rect joined into the
// bounds (Rect_Union). Each line moves the pen down the line height. The bounds copied to bounds (unless 0), the position after the last
// record the cursor. *status = 0. Returns status.
s32 *__fastcall TextLayout_Place(u8 *layout, void *, s32 *status, const u8 *style, s32 *bounds, u8 render);

// 0x6ae0f0 (thiscall (status, rect, text, style, bounds), pops 0x14): the text set (TextLayout_SetText, a failure giving *status = -1);
// without records the origin set from rect (TextLayout_SetOrigin) and that is all; otherwise the width (+0xa8) rect's, the lines arranged
// in rect (TextLayout_Arrange, a failure -1) and placed and drawn (TextLayout_Place, render 1, its status not looked at), *status = 0.
// Returns status.
s32 *__fastcall TextLayout_Draw(u8 *layout, void *, s32 *status, const s32 *rect, const u8 *text, u8 *style, s32 *bounds);

// 0x6ae1c0 (thiscall (status, rect, text, style, box), pops 0x14): as TextLayout_Draw up to the arrangement; then without box *status = -1;
// with the flag +0x94 the text placed (TextLayout_Place, no bounds, no rendering), and the text box (+0xac-+0xb8) copied to box,
// *status = 0. Returns status.
s32 *__fastcall TextLayout_Measure(u8 *layout, void *, s32 *status, const s32 *rect, const u8 *text, u8 *style, s32 *box);

}
