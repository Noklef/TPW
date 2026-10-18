// The game panel object at 0x7cc328 (built by 0x4b44d0: the panel base 0x48c9e0 with vtable 0x6ff1f0, then its own vtable 0x6ffdd8 and
// id 0x3eb; its methods 0x4b4a50, 0x4b4e30, 0x4b46a0, 0x4b4650 and 0x4b47a0 open the GUI windows 0x1e8a / 0x1e8b), with a sprite
// script slot at +0x24. Reimplementation of 0x4b4670. The names are unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x4b4670 (thiscall; slot 8 of the vtable 0x6ffdd8): when the script slot at +0x24 is set it is killed (SpriteScript_Kill) and
// +0x24 = 0, +0x28 = +0x2c = -1; returns -1 for that, else 0.
u32 __fastcall Obj_KillSlot24(u8 *object);
}  // extern "C"
