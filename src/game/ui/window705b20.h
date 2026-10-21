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

// 0x66b088 (thiscall (mode, value), pops 8; name unconfirmed): the position +0x140 moved by mode: 1 / 0 the step +0x144 added /
// taken, 2 / 3 the page +0x148 added / taken, -1 the value, -2 kept (sic: any other mode leaves it unset in the original; no caller
// passes one); clamped between +0x138 and +0x13c (signed; the first bound applied first: with the step above 0 at most +0x138 then at
// least +0x13c, else the other way round). Unchanged (and the mode not -2): 0. Else stored; the thumb (child id 3; Window_FindChild,
// its inner rectangle by Window_GetRect) placed (Window_SetRect) by bit 2 of the word +0x14c across from +0x30 to +0x34 at
// (pos - +0x13c) * room / range, else down from +0x32 to +0x36 at (+0x138 - pos) * room / range (single IDIVs; at the start for a range
// of 0), its length kept; then, with the range not 0, the objects +0x98 / +0x9c (when set) told by slot 3 (8, an event on the stack
// whose dword +0 is (pos - +0x13c) << 10 / range, its low word sign-extended), Window_Notify80 (pos), and when the position changed the
// parent +0x60 sent 0x800 (id +0x8c, pos) (Window_SendMessage). Returns 1.
u32 __fastcall Window705b20_SetPos(u8 *window, void *, s32 mode, s32 value);

// 0x66af19 (thiscall; name unconfirmed): the position +0x140 read back from the thumb (child id 3; Window_FindChild, its inner rectangle
// by Window_GetRect): by bit 2 of the word +0x14c its left's offset from +0x30, else its bottom's from +0x36, times the range +0x138 -
// +0x13c over the room left (+0x34 - +0x30, or +0x36 - +0x32, less the thumb's length; one IDIV that faults, sic, on a room of 0), plus
// +0x13c. When the position changed the parent +0x60 is posted 0x800 (id +0x8c, pos) (Window_PostMessage). EAX not kept.
void __fastcall Window705b20_PosFromThumb(u8 *window);

// 0x66b8aa (thiscall (x, y: words), pops 8; name unconfirmed): a click on the bar off the thumb pages toward it: with the thumb (child id
// 3; Window_FindChild; none: nothing) and its inner rectangle (Window_GetRect), the distance d = x - (left + (right - left)) for bit 2 of
// the word +0x14c, else (top + (bottom - top)) - y (a word either way); a page back (mode 2) for d < 0, else forward (3), swapped when
// the step +0x144 is above 0 (signed); then Window705b20_SetPos(mode, 0). EAX not kept.
void __fastcall Window705b20_PageToward(u8 *window, void *, u32 x, u32 y);

// A GUI rectangle (vtable, then the words left, top, right, bottom) passed by value: the callee destroys it.
struct GuiRectByValue {
    u8 bytes[12];
};

// 0x66b771 (cdecl (rect, bounds by value, dx, dy, across); the thumb's drag, from 0x66bc5f; name unconfirmed): the rectangle's left and
// right (words +4 / +8) moved by the word dx for across non-zero, else its top and bottom (+6 / +0xa) by dy; then kept inside the bounds:
// below their low edge, put on it with its size (as moved, signed) kept; then above their high edge, put on that the same way. The bounds
// destroyed (GuiRect_Dtor), whose EAX is left.
u32 __cdecl Window705b20_MoveInside(u8 *rect, GuiRectByValue bounds, u32 dx, u32 dy, u32 across);

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
