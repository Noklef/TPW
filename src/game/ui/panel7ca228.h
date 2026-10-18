// The game panel object at 0x7ca228 (built by 0x4997d0: the panel base 0x48c9e0 with its window at +0x10, then its own vtable 0x6ff8d0
// and +4 = 3), with an object at +0x20. Reimplementation of 0x499b40. The names are unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x499b40 (thiscall; slot 1 of the vtable 0x6ff8d0): the object +0x20's slot 9 given the panel's slot 13 (the object's vtable read before
// that call, the object itself after it), the window +0x10's child 0x1581 shown when the thing [0x7c2658] (read first) has bit 4 of its
// byte +0x32, then the panel's slots 6 and 5.
void __fastcall Panel7ca228_Update(u8 *self);

}
