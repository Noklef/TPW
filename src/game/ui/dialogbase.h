// The message handler the game's dialogs fall back on (0x47ee90, called by the handlers at 0x49e8ec, 0x49e97f, 0x49ec6d, 0x4a2768,
// 0x4a6d3f, 0x4a6e2d, 0x4b9f16, 0x4ba72b, 0x4bb05f and 0x4bb715). The name is unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x47ee90 (cdecl (window, message, a, b)): on message 0x14 the window's parent (+0x60, Fn_65d55a) is posted message 4 (0, 0)
// (Window_PostMessage) and the focus given back (Focus_Restore); then the window's default handler (Window_CallDefault) with the
// message, its result returned.
s32 __cdecl Dialog_BaseHandler(u8 *window, u32 message, u32 a, u32 b);
}
