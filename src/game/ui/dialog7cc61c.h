// The window at 0x7cc61c (made by 0x4bb800 from the script 0x7586f0 with the handler 0x4bb690, when the object at 0xf82884 is there).
// Reimplementation of its handler 0x4bb690. The names are unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x4bb690 (cdecl; window, message, a, b): on 0x14 the global 0x7cc61c cleared, the window at +4 of the object at 0xf82884 sent (6, 1, 0)
// (Window_SendMessage) and the focus given back (Focus_Restore); on 0x15 that window sent (6, 0, 0); on 0x100 with a = -1 the window at
// 0x7cc61c sent (4, 0, 0) and 0 returned. Every other case ends in Dialog_BaseHandler (window, message, a, b), whose result it returns.
s32 __cdecl Dialog7cc61c_Handler(u8 *window, u32 message, u32 a, u32 b);
}
