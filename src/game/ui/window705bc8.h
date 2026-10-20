// The GUI library's window subclass with the vtable 0x705bc8 (kind 11 of the factory 0x65dd7b: two ranges, +0x138 to +0x13c (0xfff) and
// +0x14c to +0x150 (1), with their spans +0x140 / +0x154, the handler 0x66d001; what it is is unconfirmed). Reimplementation of 0x66c970.
// The names are unconfirmed.
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
}
