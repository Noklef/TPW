// The game panel object at 0x7cc328 (built by 0x4b44d0: the panel base 0x48c9e0 with vtable 0x6ff1f0, then its own vtable 0x6ffdd8 and
// id 0x3eb; its methods 0x4b4a50, 0x4b4e30, 0x4b46a0, 0x4b4650 and 0x4b47a0 open the GUI windows 0x1e8a / 0x1e8b), with a sprite
// script slot at +0x24. Reimplementation of 0x4b4670, 0x4b47e0, 0x4b4850 and 0x4b46a0. The names are unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x4b4670 (thiscall; slot 8 of the vtable 0x6ffdd8): when the script slot at +0x24 is set it is killed (SpriteScript_Kill) and
// +0x24 = 0, +0x28 = +0x2c = -1; returns -1 for that, else 0.
u32 __fastcall Obj_KillSlot24(u8 *object);

// 0x4b47e0 (cdecl (staff); the callback 0x4b4850 sweeps the staff with): a staff member resting (state 3) in the area [0x7cc310]
// (Staff_IsState3At) is added to the list [0x7cc314] (Window7059f0_AddItem: the texts its name (Staff_GetName) and the low byte of its
// float +0x1fc truncated, times 1024 / 100; the data the staff; no parent, no refresh). Returns 1.
u32 __cdecl Panel7cc328_AddStaffRow(u8 *staff);

// 0x4b4850 (cdecl (window, message, id, b); the list window's handler, stored by 0x4b4a50): on message 0x101 with b 1 from the tabs
// 0x1e7d-0x1e81, the list 0x1e7b of the window [0x7cc338] (Window_FindChild) cleared (Window7059f0_Clear) and remembered at
// [0x7cc314], the area [0x7cc310] the thing [0x7c2658]'s handle (Thing_GetHandle), the tab [0x7cc358] 0, 4, 2, 1, 3 and the staff of
// the group 2, 0x20, 0x10, 4, 8 swept into it (Things_ForEachSelected, Panel7cc328_AddStaffRow; the original hands 0x4b47e0), the
// rows refreshed (Window7059f0_RefreshRows) and slot 8 of the panel 0x7cc328 called. Then (always) Window_CallDefault's result.
s32 __cdecl Panel7cc328_OnMessage(u8 *window, u32 message, u32 id, u32 b);

// 0x4b4e30 (thiscall; slot 1 of the vtable 0x6ffdd8): the +0x20 object's slot 9 called with the panel's slot 13's answer (that object's
// vtable read before the call, the object after), the panel's slot 6; then the list 0x1e7b of the window +0x10 (Window_FindChild, not
// checked) cleared and remembered at [0x7cc314], the area [0x7cc310] the thing [0x7c2658]'s handle, the tab [0x7cc358] (written back as
// it was) 0-4 sweeping the staff group 2, 4, 0x10, 8, 0x20 into it (Things_ForEachSelected, Panel7cc328_AddStaffRow), any other tab none;
// the rows refreshed (Window7059f0_RefreshRows), the panel's slots 8 and 5.
void __fastcall Panel7cc328_Reopen(u8 *self);

// 0x4b4f00 (thiscall (staff), pops 4; called by 0x481980 on the panel 0x7cc328): with a window +0x10, a staff member resting (state 3) in
// the area [0x7cc310] whose group index (Byte2_ToIndex) is the tab [0x7cc358] is added to that window's list 0x1e7b as
// Panel7cc328_AddStaffRow adds it, but refreshed (AddItem's last argument 1).
void __fastcall Panel7cc328_AddStaff(u8 *self, void *, u8 *staff);

// 0x4b4f90 (thiscall (staff), pops 4; called by 0x4819b4): with a window +0x10, the staff member's row in its list 0x1e7b
// (Window7059f0_FindItemData) removed (Window7059f0_RemoveItem) when there is one.
void __fastcall Panel7cc328_RemoveStaff(u8 *self, void *, u8 *staff);

// 0x481960 (thiscall (handle, added), pops 8; this, 0x7b51f0 at its one caller 0x50cb53, unused; name unconfirmed): the thing of the
// handle's low word ([0x7cfb90] + handle * 20) added to the staff panel 0x7cc328 (Panel7cc328_AddStaff) when added is set, else removed
// (Panel7cc328_RemoveStaff). (The debug print of whether the table's first slot is empty is left out.)
void __fastcall Panel7cc328_OnStaff(void *self, void *, u32 handle, u32 added);

// 0x4b46a0 (thiscall; slot 3 of the vtable 0x6ffdd8): the window +0x14's children dressed (ui/panelparts; the window and the children not
// checked, the window read again for each): 0x1e8b text 0x38, 0x1e8a a 0x6ff3b0 part with 0.
void __fastcall Panel7cc328_Dress(u8 *self);
}  // extern "C"
