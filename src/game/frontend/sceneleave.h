// Front-end scene objects leaving and stepping through their items: slot 8 of the vtables 0x702bc8, 0x702e70, 0x702ec0, 0x702dd0 and 0x702cb0, and that last class's slots 14 and 15 and 0x5e14a0.
// All thiscall, named by their vtables until the classes are known (unconfirmed). Slot 9 (+0x24) answers whether the object is active. An item (the +8 list's nodes {item, next, previous}, the current one at +0xc)
// has a pair of looped sounds at +0x1c / +0x20 and at +0x18 its entry in the table [0x803a4c] of two categories.
#pragma once

#include "../../../re/types.h"

// Their calls go to env (nullptr: the ports).
struct SceneLeaveEnv {
    void(__cdecl *stop)(u32 id);                                                   // Sound_Stop (0x51c300)
    u32(__cdecl *playAt)(u32 owner, u32 category, u32 sound, s32 x, s32 y, s32 z);  // Sound_PlayAt (0x51bfc0)
    void(__cdecl *apply)();                                                        // Sound_ApplySettings (0x51bd70)
    void(__fastcall *drain0c)(u8 *object);                                         // Obj_DrainList0c (0x5d8a10)
    void(__fastcall *drain04)(u8 *object);                                         // Obj_DrainList04 (0x5da3e0)
    void(__fastcall *release)(u8 *user);                                           // HolderUser_Release (0x5d8820)
    s32(__cdecl *message7cdb0c)();                                                 // Dialog7cdb0c_Message4 (0x4c5780)
    s32(__cdecl *message7cc4c4)();                                                 // Dialog7cc4c4_Message4 (0x4b9c00)
    s32(__cdecl *message7cc4b4)();                                                 // Dialog7cc4b4_Message4 (0x4b8e80)
    s32(__cdecl *message7cc47c)();                                                 // Dialog7cc47c_Message4 (0x4b87f0)
    void(__fastcall *setNameWords)(u8 *object, void *, u32 flag, u8 *text, const u16 *table);  // Obj_SetNameAndWords (0x5b7990)
};
void SceneLeave_SetEnv(const SceneLeaveEnv *env);

extern "C" {

// 0x5dd220: when active: +4 = 0, Dialog7cdb0c_Message4, the lists at +8 and +0x6c drained (ObjList_Drain), Obj_DrainList0c of [0xf83774], bit 0x2000000 of the render view's flags (0x8bcbc8) cleared, the sounds +0x74 and
// +0x78 stopped and cleared.
void __fastcall ViewCtl702bc8_Leave(u8 *self);
// 0x5dd690: when active: the same when still active (slot 9 asked twice), then Dialog7cc4c4_Message4.
void __fastcall ViewCtl702e70_Leave(u8 *self);
// 0x5dd950 (name invented): 0x5dd690's work, then the session's (Online_Instance) name and words cleared (Obj_SetNameAndWords(0, 0, 0)).
void __fastcall ViewCtl702e20_Leave(u8 *self);

// 0x5e1450: when active: +4 = 0, the current item's sounds (when there is one) stopped and cleared.
void __fastcall Class702cb0_Leave(u8 *self);
// 0x5e1bd0 / 0x5e2550: when active: 0x5e1450's work when still active, the list at +8 drained, HolderUser_Release of [0xf83774], Obj_DrainList04 of [0xf83784], Obj_DrainList0c of [0xf83774], slot 11 (+0x2c), then
// Dialog7cc4b4_Message4 / Dialog7cc47c_Message4.
void __fastcall Scene702ec0_Leave(u8 *self);
void __fastcall Scene702dd0_Leave(u8 *self);

// 0x5e14a0: the current item's sounds stopped and cleared; with a list (+8) of a count (+0xc) not 0 whose first item is not 0, that item's sounds played: +0x1c = Sound_PlayAt (+0x1c, the entry's second category, 1, 0, 0, 0),
// +0x20 = Sound_PlayAt (+0x20, its first, 2, 0, 0, 0), Sound_ApplySettings. +0xc is left as it was.
void __fastcall Class702cb0_PlayFirst(u8 *self);

// 0x5e1640 / 0x5e1730 (slots 14, 15): when +0x14 is 0 and the list's count not 0: the current item's sounds stopped and cleared, slot 19 (+0x4c) called with the item after / before the current one (going round; the first / last
// for none), then the sounds played of the current item as it then is, as in 0x5e14a0. sic: a current item not in the list reads through null, a fault in the original too.
void __fastcall Class702cb0_NextItem(u8 *self);
void __fastcall Class702cb0_PreviousItem(u8 *self);
}
