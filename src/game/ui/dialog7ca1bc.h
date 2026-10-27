// The dialog whose window is at 0x7ca1bc (built at start-up from the template 0x7514c0, its handler 0x497a70: on 0x100 its buttons
// 0x1b59-0x1b5b open other panels; released by Global7ca1bc_Release). Reimplementation of 0x497b20. The name is invented.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x497b20 (cdecl; name invented): the dialog built on the GUI root [0xfaa5d8] from the template 0x7514c0 with the handler 0x497a70
// (WindowScript_Build) as [0x7ca1bc], taken out of the root's children (Window_Unlink), its dword +0xa0 index 0 set to 0
// (Window_SetDwordA0) and hidden (Window_SetShown(0)); none of it checked.
void __cdecl Dialog7ca1bc_Create();
}
