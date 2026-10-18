// The two view objects at +0xc and +0x10 of the scene object [0xf85614] (Class702d78) that the 0x7cca94 dialog's buttons drive (its handler
// 0x4c5460): a step at +0x14, two values at +0x48 and +0x50 moved by it, and a mode at +0x58 choosing which vector (+0x1c or +0x28) is
// copied to +0x10 (Obj_ChooseVector, 0x5dd3d0).
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x5dd310 / 0x5dd320 / 0x5dd330 / 0x5dd340 (thiscall; names unconfirmed): the float +0x48 lowered / raised by the float +0x14, the float
// +0x50 raised / lowered by it.
void __fastcall ViewCtl_Lower48(u8 *self);
void __fastcall ViewCtl_Raise48(u8 *self);
void __fastcall ViewCtl_Raise50(u8 *self);
void __fastcall ViewCtl_Lower50(u8 *self);

// 0x5dd350 / 0x5dd390 (thiscall; names unconfirmed): the mode +0x58 set to 1 / 0, the dialog [0x7cdb0c]'s child 0x16007 sent message 6 with 0 / 1
// (Dialog7cdb0c_Child16007Message6Off / On), then the vector at +0x10 taken from +0x28 / +0x1c by the mode as it then is (Obj_ChooseVector).
void __fastcall ViewCtl_UseSecondVector(u8 *self);
void __fastcall ViewCtl_UseFirstVector(u8 *self);
}  // extern "C"
