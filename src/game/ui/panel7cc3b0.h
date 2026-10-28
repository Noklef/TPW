// The game panel object at 0x7cc3b0 (its vtable 0x6ffe50). Reimplementation of 0x4b53c0, 0x4b5330, 0x4b5a60, 0x4b6080 and 0x4b58b0. The names are unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x4b5330 (thiscall; slot 6 of the vtable 0x6ffe50): with a thing selected ([0x7c2658]), its preview shown (ThingPreview_Show) over the
// window +0x18's saved rectangle (a GuiRect on the stack: Window_GetSavedRect), at that window's depth +0xe0 (sign-extended) + 1,
// scaled 8.0.
void __fastcall Panel7cc3b0_ShowPreview(u8 *self);

// 0x4b5a60 (thiscall; slot 0 of the vtable 0x6ffe50): as Panel7ca228_Open, with the selected thing [0x7cc3a4], the children 0x50d and
// 0x50f and the hover label 0x520.
void __fastcall Panel7cc3b0_Open(u8 *self);

// 0x4b6080 (thiscall (thing), pops 4; slot 10 of the vtable 0x6ffe50): the selected thing [0x7cc3a4] = thing, the window built
// (WindowAt10_Create: the script 0x756f48, the handler 0x4b5cb0, the children 0x511 and 0x512, no inner one), the window +0x18 given the
// handler 0x4b5c80 (Window_SetHandler, not checked) and the advisor posted 0xc0 (Advisor_PostId).
void __fastcall Panel7cc3b0_Show(u8 *self, void *, u8 *thing);

// 0x4b5c80 (cdecl (window, message, a, b); the handler Panel7cc3b0_Show gives the window +0x18; name invented): on 0x10006 the camera
// centred on the shown thing (Camera_CentreOnGlobalThing); every message then to the default handler (Window_CallDefault), its answer returned.
s32 __cdecl Panel7cc3b0_WindowHandler(u8 *window, u32 message, u32 a, u32 b);

// 0x4b53c0 (thiscall; slot 3 of the vtable 0x6ffe50): with the window +0x14 set (read again for each child), its children dressed in this
// order (ui/panelparts; a text a TextPart_Ctor part with that language text, a part's or a bar's value given to the child's slot 7): 0x51a
// text 0x40, 0x51d text 0x41, 0x519 text 0x43, 0x518 text 0x45, 0x51c text 0x44, 0x516 text 0x42, 0x51b no text, 0x513 a happygrad bar
// with 0x64, 0x514 a ridestatbar bar with 0x64, 0x51e a 0x6ff3b0 part with 0, 0x515 a ridestatbar bar with 0x1f4, 0x517 no text.
void __fastcall Panel7cc3b0_Dress(u8 *self);

// 0x4b58b0 (thiscall; slot 5 of the vtable 0x6ffe50; name invented): with the window +0x14 set (read again for each child, none of
// them checked), the staff member [0x7c2658]'s stats: 0x51b sent (slot 9) the text 0x1bb (into 0x7ba058) with its age in days
// (Staff_AgeDays) as the number argument, 0x51e given (slot 7) the wage (Staff_GetWage), 0x514 the grade level * 1024 / 100
// (unsigned), 0x51f told (Window_Notify1) the code (Index_ToCode485ec0) of its +0x19c index (Mode19c_ToIndex), 0x517 given the
// language text of that index (Language_SetObjectText; 0 for 1 with no patrol region), 0x513 / 0x515 the low bytes of the floats
// +0x1f8 / +0x1fc (__ftol) * 1024 / 100 (signed).
void __fastcall Panel7cc3b0_ShowStats(u8 *self);
}
