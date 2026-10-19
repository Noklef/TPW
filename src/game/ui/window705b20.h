// The GUI library's window subclass with the vtable 0x705b20 (kind 3 of the factory 0x65dd7b, built by 0x66acf9 over Window_Ctor: a step
// +0x144 of 1, a word of flags +0x14c 1 or 2 (and 4); 0x66b557 / 0x66b604 / 0x66b6b1 make it child windows of the factory's kind 2, laid
// out by flag 2: a scroll bar with its two arrows and thumb, unconfirmed): its constructor, layout, children, range and enable.
// Reimplementation of 0x66ac00, 0x66acf9, 0x66ae83, 0x66b47d, 0x66b557, 0x66b604 and 0x66b6b1. The names are unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x66b47d (thiscall (from, to), pops 8; name unconfirmed): nothing when +0x138 is `to` and +0x13c `from` already. Otherwise the step
// +0x144 made to point from `from` to `to`: negated when it is above 0 and to < from, or below 0 and to >= from (signed); then +0x13c =
// from and +0x138 = to.
void __fastcall Window705b20_SetRange(u8 *window, void *, s32 from, s32 to);

// 0x66ae83 (thiscall (enabled: a word), pops 4; slot 5 of the vtable 0x705b20; name unconfirmed): 0 (AX) when flag 2 of +0x48 (disabled)
// already says so. Otherwise Window_SetEnabled, each child (from +0x5c, the next by Fn_65d56b) gets slot 5 (enabled), the window is
// posted 1 (0, 0) (Window_PostMessage) when the dword [0xfaa5d4] is set, and 1 (AX).
u16 __fastcall Window705b20_SetEnabled(u8 *window, void *, u32 enabled);

// 0x66ac00 (thiscall; slot 1 of the vtable 0x705b20; name unconfirmed): the saved rectangle +0x2c: with bit 4 of the word +0x14c the
// window's +8 (GuiRect_Assign); else, with bit 2, its top / bottom +0x32 / +0x36 the window's (+0xe / +0x12) and its left / right +0x30 /
// +0x34 the window's (+0xc / +0x10) moved in by their height (words); without, the other way round (left / right the window's, top /
// bottom moved in by their width).
void __fastcall Window705b20_Layout(u8 *window);

// 0x66b557 / 0x66b604 (thiscall; names unconfirmed): a child of the factory's kind 2 (flags 0, id 1 / 2) at one end of +0x2c (bit 2 of
// +0x14c: the left / right square of its height, else the top / bottom square of its width: corners from +0x30-+0x36 and +0xc-+0x12),
// shown (Window_SetShown(1)) when made.
void __fastcall Window705b20_AddArrow1(u8 *window);
void __fastcall Window705b20_AddArrow2(u8 *window);

// 0x66b6b1 (thiscall; name unconfirmed): a child of the factory's kind 2 (flags 0, id 3) from +0x30 / +0x32 to +0x30 + 0x40 / +0x36
// (bit 2 of +0x14c) or +0x34 / +0x32 + 0x40; when made, shown (Window_SetShown(1)) with the handler 0x66bb9b (Window_SetHandler).
void __fastcall Window705b20_AddThumb(u8 *window);

// 0x66acf9 (thiscall (parent, flags, id, left, top, right, bottom: words), pops 28; kind 3 of the factory 0x65dd7b; name unconfirmed):
// Window_Ctor, then the vtable 0x705b20, +0x138-+0x140 0, the step +0x144 1, the word +0x14c 1 with flags & 0x10 (else 2) and 4 with
// flags & 0x40, +0x148 1; Window705b20_Layout; with flags & 0x80 the arrows (Window705b20_AddArrow1 / 2, not with bit 4 of +0x14c) and
// the thumb (Window705b20_AddThumb); the handlers +0x114 / +0x118 0x66ba22. Returns the window.
u8 *__fastcall Window705b20_Ctor(u8 *window, void *, u8 *parent, u32 flags, u32 id, u32 left, u32 top, u32 right, u32 bottom);
}
