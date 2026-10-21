// The GUI library's window subclass with the vtable 0x705bf8 (kind 10 of the factory 0x65dd7b: dwords +0x138-+0x148 and words +0x14c /
// +0x14e, the handler 0x66d6f5: a text label, unconfirmed: the wide text +0x138 (room for +0x148 characters), the alignments +0x13c across and
// +0x140 down (1 right / bottom, 2 centred, else left / top), the font +0x4c (slot 1 the line height, slot 2 (text, first, last) a width), the
// text's place +0x14c / +0x14e). Reimplementation of 0x66d0c0, 0x66d1a6, 0x66d1f8, 0x66d386, 0x66d457, 0x66d56f, 0x66d617, 0x66d651, 0x66d678, 0x66d69f,
// 0x66d6f5 and 0x66d720.
// The names are unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x66d0c0 (thiscall (parent, flags, id, left, top, right, bottom: words), pops 28; name unconfirmed): Window_Ctor, then the vtable
// 0x705bf8, +0x148 / +0x138 / +0x13c / +0x140 / +0x144 0, the words +0x14c / +0x14e 0, the saved rectangle +0x2c set from +8
// (GuiRect_Assign) and the handler 0x66d6f5 (Window_SetHandler). Returns the window.
u8 *__fastcall Window705bf8_Ctor(u8 *window, void *, u8 *parent, u32 flags, u32 id, u32 left, u32 top, u32 right, u32 bottom);

// 0x66d1f8 (thiscall; name unconfirmed): with a font +0x4c and a text +0x138, the text's place: x +0x14c the right edge +0x34 less its width
// (font slot 2 (text, 0, wcslen - 1), a word) for +0x13c 1, the left edge +0x30 plus half (SAR) of the room left over for 2, else the left
// edge; y +0x14e the same from the bottom +0x36 / top +0x32 with the line height (slot 1) by +0x140. EAX not kept.
void __fastcall Window705bf8_Place(u8 *window);

// 0x66d386 (thiscall (text), pops 4; name unconfirmed): the x (AX) Window705bf8_Place would give that text; -1 without a font or a text.
u16 __fastcall Window705bf8_TextX(u8 *window, void *, const u16 *text);

// 0x66d457 (thiscall (text, height*), pops 8; name unconfirmed): the y (AX) for that text of 1 + its newline (10) count lines (the line height
// times that), the line height (slot 1, a word) stored through height when given; -1 without a font or a text.
u16 __fastcall Window705bf8_TextY(u8 *window, void *, const u16 *text, u16 *height);

// 0x66d56f (thiscall (text), pops 4; name unconfirmed): a text longer than the room +0x148 (signed) gets a new block (the old one freed,
// Mem_Alloc(2 * length + 2), unchecked, +0x148 the length); then with a block, the text copied in (wcscpy), placed (Window705bf8_Place)
// and 1 (AX); 0 without one.
u16 __fastcall Window705bf8_SetText(u8 *window, void *, const u16 *text);

// 0x66d617 (thiscall (out, n), pops 8; name unconfirmed): the text +0x138 copied to out (wcsncpy, n characters) when out is set.
void __fastcall Window705bf8_GetText(u8 *window, void *, u16 *out, u32 n);

// 0x66d651 / 0x66d678 (thiscall (value), pops 4; names unconfirmed): the alignment across +0x13c / down +0x140 set when it differs.
void __fastcall Window705bf8_SetAlignX(u8 *window, void *, u32 value);
void __fastcall Window705bf8_SetAlignY(u8 *window, void *, u32 value);

// 0x66d1a6 (thiscall; name unconfirmed): the destructor: the vtable 0x705bf8, the text +0x138 freed (Mem_Free) when set, then Window_Dtor,
// whose EAX is left.
u32 __fastcall Window705bf8_Dtor(u8 *window);

// 0x66d720 (thiscall (flags), pops 4; slot 2 of the vtable 0x705bf8; name unconfirmed): the destructor, the block freed for flags bit 0;
// returns the window.
u8 *__fastcall Window705bf8_DeletingDtor(u8 *window, void *, u32 flags);

// 0x66d69f (thiscall (other), pops 4; name unconfirmed): 0 when the other window's kind (slot 3) is not this one's (the other asked
// first); else wcscmp of this text +0x138 and the other's.
u32 __fastcall Window705bf8_Compare(u8 *window, void *, u8 *other);

// 0x66d6f5 (cdecl (window, message, a, b); the label's handler; name unconfirmed): Window_DefaultProc, whose answer it returns.
u32 __cdecl Window705bf8_Proc(u8 *window, u32 message, u32 a, u32 b);
}
