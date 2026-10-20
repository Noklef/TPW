// The game's dialogs: one made from a window script (0x47ed80, called by 0x4a27f0) and the message handler they fall back on (0x47ee90, called by the handlers at 0x49e8ec, 0x49e97f, 0x49ec6d, 0x4a2768,
// 0x4a6d3f, 0x4a6e2d, 0x4b9f16, 0x4ba72b, 0x4bb05f and 0x4bb715). The name is unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x47ee90 (cdecl (window, message, a, b)): on message 0x14 the window's parent (+0x60, Fn_65d55a) is posted message 4 (0, 0)
// (Window_PostMessage) and the focus given back (Focus_Restore); then the window's default handler (Window_CallDefault) with the
// message, its result returned.
s32 __cdecl Dialog_BaseHandler(u8 *window, u32 message, u32 a, u32 b);

// 0x47ed80 (cdecl (script, handler, shaded)): a frame made as a child of the GUI root [0xfaa5d8] (Window_CreateChild kind 1, flags 1,
// id 1, 0, 0 - 0x7ff, 0x5ff) and the script built in it (WindowScript_Build with the handler); the depth: shaded, the top one of the GUI
// (Gui_TopDepth(0, 0)) and the frame given the implementation named "lolight" (GuiWindows_FindByName, Window_SetImpl); else the top
// one over the built window's inner rectangle (Window_GetRect into a GuiRect on the stack). The frame at that depth + 1
// (Window_SetDepth), the built window given the handler when not 0 (Window_SetHandler) and the focus (Window_SetFocus(0)). Returns the
// built window, or 0 when the frame or the script made none.
u8 *__cdecl Dialog_Create(u8 *script, u32 handler, u32 shaded);
}
