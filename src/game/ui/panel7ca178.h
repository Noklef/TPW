// The game panel object at 0x7ca178 (its vtable 0x6ff740). Reimplementation of 0x496f90, 0x497100, 0x497990. The names are unconfirmed.
#pragma once

#include "../../../re/types.h"

// Panel7ca178_ShowStats's calls (the ports by default; the bench's recorders: their own benches have them).
struct Panel7ca178StatsEnv {
    u32(__cdecl *status)(u8 *object, s32 count);                     // Object_Status
    u32(__fastcall *canOpen)(u8 *object);                            // Object_CanOpen
    u32(__fastcall *refund)(const u8 *object);                       // Object_ScrapRefund
    u16(__fastcall *setState)(u8 *window, void *, u8 state, u8 notify);   // Window705a50_SetState
};
void Panel7ca178_SetStatsEnv(const Panel7ca178StatsEnv *env);

extern "C" {

// 0x4973a0 (thiscall; slot 5 of the vtable 0x6ff740; name invented): the shown thing's [0x7c2658] figures. Object_GetDef called, its answer unused (sic). With the window +0x14 set: 0x15bbd (when found) slot 7 given
// the newest 30 of the ring +0x1ac (head +0x224, count +0x228, wrapped byte +0x22c; dwords summed as a double, __ftol); 0x15bbb (when found) the low byte of the float +0x44 (__ftol) * 1024 / 100; 0x15bc0 (not
// checked) the scrap refund. The window +0x10 (not checked): 0x15bc4's slot 5 given 0 when the thing cannot open (Object_CanOpen), else 1 and the state 1 when +0x68 is 0 (Window705a50_SetState, notifying); 0x15bb4
// the state 1 for an Object_Status (count 2) of 0xb-0xf; the panel's +8 set to that status and its slot 9 called when that changes it. The original's debug print is left out, and so is its find of 0x15bb4 before slot 9.
void __fastcall Panel7ca178_ShowStats(u8 *self);

// 0x497100 (thiscall; slot 3 of the vtable 0x6ff740): with the window +0x14 set (read again for each child), its children dressed in this
// order (ui/panelparts; a text a TextPart_Ctor part with that language text, a part's or a bar's value given to the child's slot 7):
// 0x15bbe text 0x3c, 0x15bbc text 0x3d, 0x15bbf text 0x3f, 0x15bbd a 0x6ff350 part with 0, 0x15bbb a ridestatbar bar with 0x32, 0x15bc0 a
// 0x6ff3b0 part with 0.
void __fastcall Panel7ca178_Dress(u8 *self);

// 0x496f90 (thiscall; slot 4 of the vtable 0x6ff740): the window +0x1c dressed as panelparts::tip (the same code in four panels).
void __fastcall Panel7ca178_DressTip(u8 *self);

// 0x497990 (thiscall (thing), pops 4; slot 10 of the vtable 0x6ff740): the selected thing [0x7ca19c] = thing, the window built
// (WindowAt10_Create: the script 0x751190, the handler 0x497640, the children 0x15bb8, 0x15bba and 0x15bb9), and the advisor posted 0xd5
// when the low byte of the thing's float +0x44 (__ftol) is under 25, else 0x12b (Advisor_PostId).
void __fastcall Panel7ca178_Show(u8 *self, void *, u8 *thing);
}
