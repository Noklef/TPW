// The GUI library's window subclass with the vtable 0x705c58 (kind 6 of the factory 0x65dd7b: nothing past Window_Ctor's fields; what
// it is is unconfirmed). Reimplementation of 0x66d9e0. The names are unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x66d9e0 (thiscall (parent, flags, id, left, top, right, bottom: words), pops 28; name unconfirmed): Window_Ctor, then the vtable
// 0x705c58. Returns the window.
u8 *__fastcall Window705c58_Ctor(u8 *window, void *, u8 *parent, u32 flags, u32 id, u32 left, u32 top, u32 right, u32 bottom);

// 0x66da4d (slot 7 of 0x705c58; thiscall (value), pops 4; name unconfirmed): the value kept at +0x13c, the objects +0x98 and +0x9c (when
// set) told through their slot 3 (1, an event on the stack whose only word set, +4, is the value), then Window_Notify80(value). 1 in AX
// (the rest of EAX Window_Notify80's).
u32 __fastcall Window705c58_SetValue(u8 *window, void *, u32 value);
}
