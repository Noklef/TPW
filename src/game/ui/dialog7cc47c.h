// The window at 0x7cc47c (built from the script 0x757d88 with the handler 0x4b86a0): two yellow text labels. Reimplementation of
// 0x4b8520. The name is unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x4b8520 (cdecl): unless [0x7cc47c] is up or the frame [0xf82884] has no window +4, the script built on that window (WindowScript_Build)
// made [0x7cc47c]; its child 0x49d9 given a new TextPart6ff530 (none when the allocation fails), font 5, Window_NotifyC00(1, 1), the
// text 0 (slot 9) and the yellow (HoverLabel_SetYellow), then font 0 and the colours (0xfc, 0xe8, 7, 0xff) (Window_Notify40000); the
// child 0x49da (looked up from [0x7cc47c] read again) a new TextPart6ff530, font 0, Window_NotifyC00(1, 1) and those colours. The
// original checks neither child.
void __cdecl Dialog7cc47c_Create();

}  // extern "C"
