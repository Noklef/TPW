// The dialog whose window is at 0x7ca1e0 (built at start-up from the template 0x751720, its handler 0x498ad0: on
// 0x100 its buttons 0x63 / 0x64 act and the sound 0x131 plays; on 0x14 [0x7ca1e0] forgotten). Reimplementation of 0x498b50. The name is
// invented.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x498b50 (cdecl; name invented): the dialog built on the GUI root [0xfaa5d8] from the template 0x751720 with the handler 0x498ad0
// (WindowScript_Build) as [0x7ca1e0], taken out of the root's children (Window_Unlink), its dword +0xa0 index 0 set to 1
// (Window_SetDwordA0) and hidden (Window_SetShown(0)); none of it checked.
void __cdecl Dialog7ca1e0_Create();
}
