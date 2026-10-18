// The text layout engine (0x6ad000-0x6b1000) the GUI's text drawing (0x492750, 0x4929f0) builds on the stack: three nested objects.
// - The text style (vtable 0x70a0d8, 0x50 bytes; the global one at 0x78ba2c): +4 the font (a reference-counted object: slot 0 releases,
//   slot 1 adds a reference, slot 9 answers its flags), +8 the font's flags, +0xc a state (3 after construction or a font change, 1 after a
//   colour), +0x10 1, +0x14 a SurfaceInfo (0x28 bytes), +0x3c, +0x40, +0x44 the colour (5 bytes), +0x4c.
// - The style holder (vtable 0x70a13c, 0x58 bytes): +4 the style, +0x54.
// - The layout (vtable 0x70a10c, 0xbc bytes): +4 the holder; +0x5c, +0x70 and +0x84 one byte each before three vectors (begin, end, capacity
//   at +0x60, +0x74, +0x88), +0x94 a flag, +0x98, +0x9c, +0xa0 / +0xa4 two kinds, +0xac-+0xb8.
// Reimplementation of 0x6ad5d0, 0x6ad620, 0x6ad640, 0x6ad6e0, 0x6ad7c0, 0x6ade00, 0x6ade90, 0x6adeb0, 0x6b0230, 0x6b0250 and 0x6b0270.
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

}
