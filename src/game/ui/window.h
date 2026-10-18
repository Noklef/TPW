// The GUI library's windows (0x65xxxx): a window's children are a list from +0x5c linked by +0x54, each with its id at +0x8c
// (Fn_65d7bc). Reimplementation of 0x65d7d0.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x65d7d0 (thiscall (id), pops 4; name unconfirmed): the first child of the window whose id is `id`, 0 when none is.
u8 *__fastcall Window_FindChild(u8 *window, void *, u32 id);

// 0x65f697 (thiscall (message, a, b), pops 12; name unconfirmed): the window's handler (the cdecl function at +0x114) called with
// (window, message, a, b) and its result returned; -1 when there is none.
s32 __fastcall Window_CallHandler(u8 *window, void *, u32 message, u32 a, u32 b);

// 0x658d9f (cdecl (window, message, a, b); name unconfirmed): -1 for no window or one whose byte +4 (Fn_659710) is set; otherwise the
// global message hook [0xfaa5fc] (when set) is called with the same four, then Window_CallHandler's result is returned.
s32 __cdecl Window_SendMessage(u8 *window, u32 message, u32 a, u32 b);

// 0x65d6db (thiscall (a, b), pops 8; name unconfirmed): slot 7 (thiscall (a, b)) of the window's object at +0x9c, its result; 0 when there is
// none.
u32 __fastcall Window_ImplSlot7(u8 *window, void *, u32 a, u32 b);

// 0x65d745 (thiscall; name unconfirmed): slot 6 (thiscall, no arguments) of the window's object at +0x9c, its result; the window itself when
// there is none (sic: EAX still holds it).
u32 __fastcall Window_ImplSlot6(u8 *window);

// 0x65e705 (thiscall (index: a word), pops 4; name unconfirmed): the dword +0xa0 + 4 * index (signed) for an index below 16, else 0.
u32 __fastcall Window_DwordA0(u8 *window, void *, u32 index);
}
