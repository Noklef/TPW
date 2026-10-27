// The game panel object at 0x7cc298 (built by 0x4b0eb0: the panel base 0x48c9e0 with its window at +0x10, then its own vtable 0x6ffce8
// and +4 = 2), with an object at +0x20; it shows the thing [0x7c2658]. Reimplementation of 0x4b0ed0, 0x4b1ee0, 0x4b1040 and 0x4b20f0. The names are unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x4b1ee0 (thiscall; slot 1 of the vtable 0x6ffce8): the object +0x20's slot 9 given the panel's slot 13 (the object's vtable read before
// that call, the object itself after it), then the panel's slots 6 and 5. In the window +0x10 (read for each lookup, Window_FindChild): the
// 0x705a50 child 0xa08a (when there is one) set to the state Is68_Zero_4e0440 of the thing [0x7c2658] (read first), notifying; the children
// 0xa08e / 0xa08f / 0xa090 (not checked) given their slot 7 the thing's byte +0x190 / dword +0x194 / dword +0x188. Then with the dword
// +0x10c of the thing's definition (Object_GetDef) set, the children 0xa090, 0xa08e, 0xa0a6 and 0xa0a7 hidden (Window_SetShown(0)) and the
// child 0xa0a5's slot 9 given the text 0x34 (Language_WideText); else those shown and the text 0x33. (Each child's vtable is read before
// the value it is given is.)
void __fastcall Panel7cc298_Update(u8 *self);

// 0x4b20f0 (cdecl (thing), the callback Things_Select (0x481bc0) is given for the group 0x100 by the panel's handler; name invented): a
// thing of the shown thing's [0x7c2658] kind (+0xe) given +0x194 [0x7cc2c4] and, when its definition's +0x10c is 0, the chance of
// winning the byte [0x7cc2bc] and +0x188 [0x7cc2c0]. 1.
u32 __cdecl Panel7cc298_ApplyToThing(u8 *thing);

// 0x4b1040 (thiscall; slot 3 of the vtable 0x6ffce8): with the window +0x14 set (read again for each child), its children dressed in the
// original's order (ui/panelparts): 0xa09c-0xa09f texts 0x29-0x30, 0xa09d a text with none, bars (ridestatbar) 0xa096 / 0xa099 / 0xa094
// with 0x32 / 0x64 / 0x32, 0x6ff3b0 parts 0xa093 / 0xa092 / 0xa0a0 with 0, and 0xa09e a happygrad bar with 0x64.
void __fastcall Panel7cc298_Dress(u8 *self);

// 0x4b0ed0 (thiscall; slot 4 of the vtable 0x6ffce8): the window +0x1c dressed as panelparts::tip (the same code in four panels).
void __fastcall Panel7cc298_DressTip(u8 *self);
}
