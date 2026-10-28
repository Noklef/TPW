// The game panel object at 0x7ca228 (built by 0x4997d0: the panel base 0x48c9e0 with its window at +0x10, then its own vtable 0x6ff8d0
// and +4 = 3), with an object at +0x20. Reimplementation of 0x499b40, 0x499890, 0x499af0 and 0x499eb0. The names are unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x499b40 (thiscall; slot 1 of the vtable 0x6ff8d0): the object +0x20's slot 9 given the panel's slot 13 (the object's vtable read before
// that call, the object itself after it), the window +0x10's child 0x1581 shown when the thing [0x7c2658] (read first) has bit 4 of its
// byte +0x32, then the panel's slots 6 and 5.
void __fastcall Panel7ca228_Update(u8 *self);

// 0x499af0 (thiscall; slot 0 of the vtable 0x6ff8d0): when the group counts under 2 (Things_CountGroup of the selected thing [0x7ca218]),
// the window +0x10's children 0x157c and 0x157d hidden (Window_SetShown(0) of Window_FindChild's, not checked); then the child 0x157f
// bound as the hover label (HoverLabel_Bind).
void __fastcall Panel7ca228_Open(u8 *self);

// 0x499eb0 (thiscall (thing), pops 4; slot 10 of the vtable 0x6ff8d0; name invented): the selected thing [0x7ca218] = thing, the
// window built (WindowAt10_Create: the script 0x751a68, the handler 0x499bb0, the children 0x157e and 0x1583, no inner one), then the advisor posted 0x12a (Advisor_PostId).
void __fastcall Panel7ca228_Show(u8 *self, void *, u8 *thing);

// 0x499890 (thiscall; slot 3 of the vtable 0x6ff8d0): with the window +0x14 set (read again for each child), those of its children that
// are there dressed in this order (ui/panelparts; nothing allocated for a missing one): 0x1587 text 0x35, 0x1586 a 0x6ff350 part with 0,
// 0x1585 text 0x36, 0x1584 a 0x6ff3b0 part with 0.
void __fastcall Panel7ca228_Dress(u8 *self);

// 0x499a70 (thiscall; slot 5 of the vtable 0x6ff8d0; name invented): for the shown thing [0x7c2658] (Object_GetDef called, its answer
// unused), when the window +0x14 is set: its child 0x1586, if there, is given through its slot 7 the +0x18 of the thing's control record
// (ObjectControls_GetWord18 of its word +0xe; GameTable7cfb90_Get1da720_519510 of [0x7cf83c] called first for the ECX of that stdcall);
// then its child 0x1584 (Window_FindChild of +0x14 read again, unchecked) is given the thing's scrap refund (Object_ScrapRefund) the
// same way. Each child's vtable is read before its value is worked out.
void __fastcall Panel7ca228_ShowValues(u8 *self);

}
