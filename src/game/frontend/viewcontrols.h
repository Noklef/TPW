// The two view objects at +0xc (Class702e70) and +0x10 (Class702e20) of the scene object [0xf85614] (Class702d78, built at 0x5e3dc0) that
// the 0x7cca94 dialog's buttons drive (its handler 0x4c5460): a step at +0x14, two values at +0x48 and +0x50 moved by it, and a mode at
// +0x58 choosing which vector (+0x1c or +0x28) is copied to +0x10 (Obj_ChooseVector, 0x5dd3d0). Also the scene's +8 object (Class702dd0)
// and the dialogs these show and hide.
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

// 0x5dd7a0 (Class702e70 slot 17; thiscall, self unused; name unconfirmed): the dialogs [0x7cdb0c] and [0x7cc4c4] sent message 6 (1, 0), then
// the first's child 0x16007 (Dialog7cdb0c_Message6On, Dialog7cc4c4_Message6On, Dialog7cdb0c_Child16007Message6On, whose result is returned).
s32 __fastcall ViewCtl702e70_ShowDialogs(u8 *self);

// 0x5dda70 (Class702e20 slot 17; thiscall, self unused; name unconfirmed): the same without the child (the second send's result).
s32 __fastcall ViewCtl702e20_ShowDialogs(u8 *self);

// 0x5e2620 (Class702dd0 slot 10; thiscall; name unconfirmed): the dialog [0x7cc47c] sent message 6 (1, 0) (Dialog7cc47c_Message6On; its
// result returned), then +0xe8 = 1.
s32 __fastcall Scene702dd0_ShowDialog(u8 *self);

// 0x5e2640 (Class702dd0 slot 11; thiscall, self unused; name unconfirmed): Dialog7cc47c_Message6Off (its result).
s32 __fastcall Scene702dd0_HideDialog(u8 *self);

// 0x5e2650 / 0x5e2660 (cdecl; names unconfirmed): the two callbacks Class702dd0's slot 17 (0x5e26a0) hands 0x4c0ae0 with itself.
// 0x5e2650: Dialog7cc47c_Message6On (its result). 0x5e2660 (accepted, self): accepted 0 calls the self's slot 0x44 (no arguments; its
// result); otherwise, when +0x14 is not 0, a 2 there first plays the +0xc object's +8 lobby model (LobbyModel_Play (1, 0)) and +0x14
// becomes 0; then Dialog7cc47c_Message6On (its result).
s32 __cdecl Scene702dd0_ShowDialogCallback();
s32 __cdecl Scene702dd0_DialogDone(s32 accepted, u8 *self);
}  // extern "C"
