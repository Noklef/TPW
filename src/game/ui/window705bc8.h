// The GUI library's window subclass with the vtable 0x705bc8 (kind 11 of the factory 0x65dd7b: two ranges, +0x138 to +0x13c (0xfff) and
// +0x14c to +0x150 (1), with their spans +0x140 / +0x154, the handler 0x66d001; what it is is unconfirmed). Reimplementation of 0x66c970,
// 0x66cad3, 0x66cb69, 0x66cc4d, 0x66cc90, 0x66cd05, 0x66cd83, 0x66cdc4, 0x66cea9, 0x66cf17, 0x66cf7d and 0x66d001. The names are unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x66c970 (thiscall (parent, flags, id, left, top, right, bottom: words), pops 28; name unconfirmed): Window_Ctor, then the vtable
// 0x705bc8, +0x144 / +0x148 0, the range +0x138 0 to +0x13c 0xfff and +0x14c 0 to +0x150 1 with their spans +0x140 / +0x154, +0x158 /
// +0x15c 0, the saved rectangle +0x2c set from +8 (GuiRect_Assign), the word +0x164 0 and the handler 0x66d001 (Window_SetHandler). Returns
// the window.
u8 *__fastcall Window705bc8_Ctor(u8 *window, void *, u8 *parent, u32 flags, u32 id, u32 left, u32 top, u32 right, u32 bottom);

// 0x66cad3 (thiscall (low, high: dword pointers, each may be 0), pops 8; name unconfirmed): the first range's ends +0x138 / +0x13c set from
// those given; when the low end is above the high one (signed) both become the high end (sic: the low end saved for the swap is never
// used); then the span +0x140 = high - low. EAX is left the window.
u8 *__fastcall Window705bc8_SetRange(u8 *window, void *, const s32 *low, const s32 *high);

// 0x66cb69 (thiscall (low, high: dword pointers, each may be 0), pops 8; name unconfirmed): the second range's ends +0x14c / +0x150 set
// as Window705bc8_SetRange sets the first's (the same sic when the low end is above the high one), then the value +0x158 clamped into
// them (signed: raised to the low end, else lowered to the high one) and the span +0x154 = high - low. EAX is left the window.
u8 *__fastcall Window705bc8_SetRange2(u8 *window, void *, const s32 *low, const s32 *high);

// 0x66cc90 (thiscall; name unconfirmed): the first range's value +0x144 in the second range: its high end +0x150 when the value is the
// first's high end +0x13c, its low end +0x14c when it is the first's low end +0x138, else span2 * value / span1 + low2 (signed; not
// value - low1, sic).
s32 __fastcall Window705bc8_FirstToSecond(u8 *window);

// 0x66cd05 (thiscall; name unconfirmed): the second range's value +0x158 in the first: +0x13c at the second's high end, +0x138 at its
// low end, else (value - low2) * span1 / span2 + low1 (signed).
s32 __fastcall Window705bc8_SecondToFirst(u8 *window);

// 0x66cd83 (thiscall (other), pops 4; name unconfirmed): when the other window's slot 3 answers what this one's does (the other's asked
// first), this one's second value +0x158 less the other's; else 0.
s32 __fastcall Window705bc8_Compare(u8 *window, void *, u8 *other);

// 0x66cdc4 (thiscall (value), pops 4; name unconfirmed; called by 0x66cc5b and 0x66cf0c): the value held to the second range (lowered to
// its high end +0x150, else raised to its low end +0x14c; signed); when it differs from the value +0x158: the old one to +0x15c, the new one
// to +0x158, +0x148 its place in the first range (Window705bc8_SecondToFirst); the object +0x9c (when set) told 4 with an event whose word
// +8 is +0x148's low word (the rest the original's stack leftovers: here 0) (slot 3); and the parent +0x60 posted 0x800 (the id +0x8c, the
// value) (Window_PostMessage).
void __fastcall Window705bc8_SetValue(u8 *window, void *, s32 value);
// 0x66cc4d (thiscall (value), pops 4; name unconfirmed): Window705bc8_SetValue, then the first value +0x144 = +0x148; 1 in AX (the rest of
// EAX +0x148's).
u32 __fastcall Window705bc8_SetValueSync(u8 *window, void *, s32 value);

// 0x66cea9 (thiscall (value), pops 4; name unconfirmed; called by 0x66cff6): the value held to the first range (raised to its low end
// +0x138, else lowered to its high end +0x13c; signed); when it differs from +0x144 it becomes it and Window705bc8_SetValue is given its
// place in the second range (Window705bc8_FirstToSecond).
void __fastcall Window705bc8_SetFirst(u8 *window, void *, s32 value);

// 0x66cf17 (thiscall (on, y: words), pops 8; name unconfirmed; called by 0x66d047 / 0x66d05d): with on (its low word) the drag starts:
// the word +0x164 = 1, the words +0x160 and +0x162 the y, the mouse captured (Window_CaptureMouse); else +0x164 = 0 and the mouse released
// (Window_ReleaseMouse).
void __fastcall Window705bc8_SetDrag(u8 *window, void *, u32 on, u32 y);

// 0x66cf7d (thiscall (y), pops 4; name unconfirmed; called by 0x66d06d): while dragging (+0x164) and the y's low word (unsigned) is not
// the word +0x160 (signed, sic): +0x162 the old +0x160, +0x160 the y, and the first value moved by (old - new) * 8 (Window705bc8_SetFirst).
void __fastcall Window705bc8_Drag(u8 *window, void *, u32 y);

// 0x66d001 (cdecl (window, message, a, b); the handler the constructor sets; name unconfirmed): 0x10005: the window activated
// (Window_Activate) and, with a 0, the drag started at b (Window705bc8_SetDrag); 0x10004: with a 0, the drag ended; both return 0.
// 0x10003: dragged to a (Window705bc8_Drag); it and the rest then go to Window_DefaultProc, whose answer is returned.
u32 __cdecl Window705bc8_OnMessage(u8 *window, u32 message, u32 a, u32 b);
}
