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

// 0x658ccc (cdecl (window, message, a, b); name unconfirmed): queues the message (MsgQueue_Push, ui/msgqueue). Messages 1-3 go to the
// queue [0xfaa5f4] and 0x1000a-0x1000c to [0xfaa5f0] without any check (sic: no queue faults) and 0 returned; the others to [0xfaa5f0].
// Messages 1-3 and the others need the queue, a window and its byte +4 (Fn_659710) clear: then 1, else 0 (the push's own result is
// dropped).
u32 __cdecl Window_PostMessage(u8 *window, u32 message, u32 a, u32 b);

// 0x65db25 (thiscall (x, y: words), pops 8; name unconfirmed): the deepest window under the point. 0 for a window with flag 2 of +0x48 or
// its byte +4 set, or with neither flag 0 nor any of the top four. Otherwise each child (from +0x5c by +0x54) is searched in turn, the
// last one found kept; then slot 0 (x, y) of the rectangle at +0x44 (GuiRect_Contains) is always asked, and the window itself is the
// answer when that holds and no child was found.
u8 *__fastcall Window_HitTest(u8 *window, void *, u32 x, u32 y);

// 0x6589f9 (cdecl; name unconfirmed): the hover update: 0 in AX without the root window [0xfaa5d8] or with [0xfaa5e8] set. Else the
// window under the mouse (the words 0xfaa540, 0xfaa544; Window_HitTest on the root): when it is not the hovered one [0xfaa5e0], that
// one (when set) is posted 0x10002 (0, 0), the new one becomes it and is posted 0x10001 (x << 16 | y, both signed: sic, a negative y
// fills the top half, 0) (Window_PostMessage). 1 in AX. Only AX is set.
u32 __cdecl Window_UpdateHover();
}
