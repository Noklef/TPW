// The game panel object at 0x7cc3e0 (built by 0x4b6470: its vtable 0x6ffec8). Reimplementation of 0x4b6540 and 0x4b67a0. The names are unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x4b6540 (thiscall; slot 3 of the vtable 0x6ffec8): as Panel7ca228_Dress, with the children 0x5211 text 0x37, 0x5210 a 0x6ff350 part
// with 0, 0x520f text 0x38, 0x520e a 0x6ff3b0 part with 0.
void __fastcall Panel7cc3e0_Dress(u8 *self);

// 0x4b67a0 (thiscall; slot 0 of the vtable 0x6ffec8): as Panel7ca228_Open, with the selected thing [0x7cc3d8], the children 0x520a and
// 0x520b and the hover label 0x5212.
void __fastcall Panel7cc3e0_Open(u8 *self);

}  // extern "C"
