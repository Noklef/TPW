// The GUI library's window subclass with the vtable 0x705af0 (kind 12 of the factory 0x65dd7b: dwords +0x138-+0x148, +0x140 the address
// 0x9708cd, the handler 0x66aa61; what it is is unconfirmed: a value +0x140 kept in the range +0x138-+0x13c, the one before +0x144, a step
// +0x148, the parent posted 0x800 on a change, a slider going by those). Reimplementation of 0x66a4c0, 0x66a598, 0x66a5c4, 0x66a68d,
// 0x66a6db and 0x66a71c. The names are unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x66a4c0 (thiscall (parent, flags, id, left, top, right, bottom: words), pops 28; name unconfirmed): Window_Ctor, then the vtable
// 0x705af0, +0x138 0, +0x13c 1, +0x140 0x9708cd, +0x144 0, +0x148 1, the byte +0x154 1, the saved rectangle +0x2c set from +8
// (GuiRect_Assign) and the handler 0x66aa61 (Window_SetHandler). Returns the window.
u8 *__fastcall Window705af0_Ctor(u8 *window, void *, u8 *parent, u32 flags, u32 id, u32 left, u32 top, u32 right, u32 bottom);

// 0x66a598 (thiscall; name unconfirmed): the destructor: the vtable 0x705af0, then Window_Dtor, whose EAX is left.
u32 __fastcall Window705af0_Dtor(u8 *window);

// 0x66a5c4 (thiscall (low, high: pointers to dwords, each may be 0), pops 8; name unconfirmed): +0x138 = *low, +0x13c = *high (those
// given); when +0x138 > +0x13c (signed) both become +0x13c (sic: a swap that loses the low end); then the value +0x140 clamped into
// them (below +0x138 to it, else above +0x13c to it). EAX not kept.
void __fastcall Window705af0_SetRange(u8 *window, void *, const s32 *low, const s32 *high);

// 0x66a68d (thiscall (step: a pointer to a dword, may be 0), pops 4; name unconfirmed): +0x148 = *step when given.
void __fastcall Window705af0_SetStep(u8 *window, void *, const s32 *step);

// 0x66a6db (thiscall (other), pops 4; name unconfirmed): the other's slot 3 (no arguments) and then the window's; when they answer the
// same, the window's +0x140 less the other's, else 0.
s32 __fastcall Window705af0_Distance(u8 *window, void *, u8 *other);

// 0x66a71c (thiscall (value), pops 4; name unconfirmed): the value clamped (above +0x13c to it, else below +0x138 to it; signed); when it
// is +0x140 already: 0 (AX) unless the byte +0x154 is set. Otherwise the byte +0x154 cleared, +0x144 = +0x140, +0x140 = value, Window_Notify80(value),
// the parent +0x60 posted 0x800 (the id +0x8c, the value; Window_PostMessage); 1 (AX).
u16 __fastcall Window705af0_SetValue(u8 *window, void *, s32 value);
}
