// The thing browser's panel (the thing shown [0x7cc308], the panel window [0x7cc304]): its labels filled for the thing, and the handler of
// its windows' radio groups. Reimplementation of 0x4b3490, 0x4b3580 and 0x4b3b90. The names are unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x4b3490 (cdecl; name unconfirmed): with a thing shown, the panel's child 0x147 given (its slot 9) the thing's name for staff (kinds
// 4-8: Staff_GetName), else the text 0x1c9 (a count +0x1d8 of 0) or 0x1c8 with the count as argument 0x18 (TextFormat into the buffer
// 0x7ba058, 0x3fff characters, the table [0x78ba94]); then the detail line (ThingBrowser_ShowDetail) and the camera on the thing
// (Camera_CentreOnSprite).
void __cdecl ThingBrowser_ShowName();

// 0x4b3580 (cdecl; name unconfirmed): with a thing shown: for staff (kinds 4-8) its mode's code (Index_ToCode485ec0 of Mode19c_ToIndex),
// when not the one shown last [0x756b98], kept there and told to the child 0x148 (Window_Notify1); for the others the child 0x14a's slot
// 7 given the count +0x30 less 1, or 0x29a for none.
void __cdecl ThingBrowser_ShowDetail();

// 0x4b3b90 (cdecl (window, message, a, b); the handler 0x4b3cf0 gives the child 0x95d of the browser's windows [0x7cc2fc] / [0x7cc300], a
// radio group; name unconfirmed): message 0x100: the group's child a (Window_FindChild) found, a selected (RadioGroup_Select), for b other
// than 1 the child set to state 1, notifying (Window705a50_SetState(1, 1); not checked for 0), and the group's parent (+0x60, Fn_65d55a)
// posted 0x101 (a, 1); 1 returned. Other messages: Window_CallDefault's result.
u32 __cdecl ThingBrowser_GroupHandler(u8 *group, u32 message, u32 a, u32 b);
}
