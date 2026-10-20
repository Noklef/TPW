// The thing browser's panel (the thing shown [0x7cc308], the panel window [0x7cc304]): its labels filled for the thing. Reimplementation
// of 0x4b3490 and 0x4b3580. The names are unconfirmed.
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
}
