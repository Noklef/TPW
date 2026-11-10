// The two view objects at +0xc (Class702e70) and +0x10 (Class702e20) of the scene object [0xf85614] (Class702d78, built at 0x5e3dc0) that
// the 0x7cca94 dialog's buttons drive (its handler 0x4c5460): a step at +0x14, two values at +0x48 and +0x50 moved by it, and a mode at
// +0x58 choosing which vector (+0x1c or +0x28) is copied to +0x10 (Obj_ChooseVector, 0x5dd3d0). Also the scene's +8 object (Class702dd0)
// and the dialogs these show and hide.
#pragma once

#include "../../../re/types.h"

// Class702d78_MakeViews's calls go to env (nullptr: the ports).
struct SceneViewsEnv {
    u8 *(__fastcall *segmentsCtor)(u8 *object);  // RandomSegments_Ctor (0x5dfcd0)
    u8 *(__fastcall *viewCtor)(u8 *object);      // Class702bc8_Ctor (0x5dc580)
};
void SceneViews_SetEnv(const SceneViewsEnv *env);

// What ViewCtl702bc8_Update calls (the port by default; the bench records it).
struct ViewCtlEnv {
    void(__cdecl *showThing)(u8 *thing);  // Dialog7cc4c4_ShowThing (0x4b9980)
    void(__cdecl *openRooms)();           // Dialog7cdac4_Open (0x4c4670)
};
void ViewCtl_SetEnv(const ViewCtlEnv *env);

extern "C" {

// 0x5e3dc0 (thiscall; name invented, unconfirmed): the scene's five objects made, each Mem_Alloc'd (0 when that failed): +4 (0xe8 bytes; RandomSegments_Ctor, vtable 0x702ec0), +0xc and +0x10
// (0x7c bytes each; Class702bc8_Ctor, vtables 0x702e70 and 0x702e20), +8 (0xec bytes; RandomSegments_Ctor, vtable 0x702dd0), +0x14 (0x2c bytes: vtable 0x702d48, +4 0, added to the scene
// [0xf85614] by its slot 0x18, +0xc..+0x14 0, +8 0x702dc8, vtable 0x702da0). Then each item of the list (+0x18 -> +4: nodes of item, next) has its slot 4 called.
void __fastcall Class702d78_MakeViews(u8 *self);

// 0x5dc860 (thiscall; name invented, unconfirmed; a virtual slot of the vtable 0x702bc8): with +0x68 set, the thing under the mouse picked (+0x70, its point +0x5c) and shown. The mouse at the
// screen's edges (byte 0x78d910 clear), or dragged (set, the input's flag 2), moves +0x48 and +0x50; these are damped by +0x18 and turn +0x44 and +0x4c, the latter held within +-(+0x40), its
// speed +0x50 then 0. +0x54 and +0x34 ease towards their targets; the camera [0xf83778] gets its position (+8) from the angles at the distance +0x54, its +0x28, +0x2c from +0x34.
void __fastcall ViewCtl702bc8_Update(u8 *self);

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
// 0x5dd7b0 (thiscall, self unused; name invented, unconfirmed): the message 6 off to the dialog [0x7cdb0c], to [0x7cc4c4], then to the first's child 0x16007 with its answer.
s32 __fastcall ViewCtl702e70_HideDialogs(u8 *self);
// 0x5dd830 (thiscall; slot of the class 0x702e70; name invented, unconfirmed): ViewCtl702bc8_Update.
void __fastcall ViewCtl702e70_Update(u8 *self);

// 0x5dda70 (Class702e20 slot 17; thiscall, self unused; name unconfirmed): the same without the child (the second send's result).
s32 __fastcall ViewCtl702e20_ShowDialogs(u8 *self);
// 0x5dda80 (thiscall, self unused; name invented, unconfirmed): the message 6 off to the dialogs [0x7cdb0c] and [0x7cc4c4].
void __fastcall ViewCtl702e20_HideDialogs(u8 *self);

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

// 0x5dd3d0 (thiscall; UseFirst / UseSecond end with it): the three dwords at +0x10 take those at +0x28 when +0x58 is set, else those at +0x1c.
void __fastcall Obj_ChooseVector(u8 *object);

// 0x5dced0 (Class702bc8 slot 5; thiscall (message, key, unused), pops 0xc; name invented): message 0x1000a with key 0x2500 / 0x2600 /
// 0x2700 / 0x2800: ViewCtl_Lower48 / Raise50 / Raise48 / Lower50 (inline in the original); message 0x10006 with 2: the mode swapped
// (ViewCtl_UseFirstVector when +0x58 is set, else ViewCtl_UseSecondVector: the original inlines the vector's copy) and the dialog's child
// 0x16006 set to it (Dialog7cdb0c_SetChild16006State). EAX not kept. 0x5dd530 (Class702e70 slot 5; name invented): the same, then for
// message 0x10005 with key 0 and an object at +0x70: [0xf82888] = its +0xc, the view's slot 8 (+0x20) and the scene's ([0xf85614]) +8
// object's slot 7 (+0x1c) called.
void __fastcall ViewCtl702bc8_Message(u8 *self, void *, u32 message, u32 key, u32 unused);
void __fastcall ViewCtl702e70_Message(u8 *self, void *, u32 message, u32 key, u32 unused);
// 0x5dd840 (Class702e20 slot 5; name invented): 0x5dced0's, then for message 0x10005 with key 0 and an object at +0x70: [0xf82888] = its +0xc, Dialog7cdac4_Open.
void __fastcall ViewCtl702e20_Message(u8 *self, void *, u32 message, u32 key, u32 unused);

// 0x5dd810 (Class702e70 slot 16; stdcall; object, pops 4): 1 when the signed dword at +0x40 is above 0, else 0.
u32 __stdcall Obj_Positive40(const u8 *object);

// 0x5ddac0 (Class702e20 slot 16; stdcall; object, pops 4): 1 when the dword at +0x60 is 0 and the signed +0x40 is below +0x64.
u32 __stdcall Obj_Pending(const u8 *object);

// 0x5e2750 (Class702dd0 slot 19; thiscall; value, pops 4): with a value, +0xc = value and +0x14 = 0.
void __fastcall Obj_SetOrKeepC(u8 *object, void *, u32 value);
}  // extern "C"

#include "../core/gamedata.h"

// Globals by address (tools/gamedata_codemod.py; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(const float, gViewcontrols_702b8c, 0x00702b8c, 0x4);
GAME_DATA(const float, gViewcontrols_702b90, 0x00702b90, 0x4);
GAME_DATA(const float, gViewcontrols_702b94, 0x00702b94, 0x4);
GAME_DATA(const float, gViewcontrols_702b98, 0x00702b98, 0x4);
GAME_DATA(const float, gViewcontrols_702b9c, 0x00702b9c, 0x4);
GAME_DATA(const float, gViewcontrols_702ba0, 0x00702ba0, 0x4);
GAME_DATA(const float, gViewcontrols_702ba4, 0x00702ba4, 0x4);
GAME_DATA(const float, gViewcontrols_702ba8, 0x00702ba8, 0x4);
GAME_DATA(const float, gViewcontrols_702bac, 0x00702bac, 0x4);
GAME_DATA(const float, gViewCtl702bc8_Update_702bb0, 0x00702bb0, 0x4);
