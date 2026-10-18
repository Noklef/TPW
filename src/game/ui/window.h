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

// 0x6616b0 (thiscall; name unconfirmed): a window's object (+0x98) let go: its word count +0x12 decremented, its slot 5 called with
// (0), then the count less (1 - the word +0x10) (both signed) returned; -1 tells the caller to delete it.
s32 __fastcall WindowImpl_Release(u8 *impl);

// 0x65d773 (thiscall; name unconfirmed): 1 in AX when the window and every parent (+0x60) up the chain has flag 0 of +0x48, else 0
// (1 for no window). Only AX is set.
u16 __fastcall Window_IsShown(u8 *window);

// 0x65bc90 (thiscall; name unconfirmed): the rectangle at +8 copied to the one at +0x2c (GuiRect_Assign); returns +0x2c.
u8 *__fastcall Window_SaveRect(u8 *window);

// 0x65c979 (thiscall (rect), pops 4; name unconfirmed): for each parent up the chain (+0x60), the rectangle's slot 2 (GuiRect_Offset)
// called with the parent's words +0xc and +0xe (sic: pushed as dwords whose upper halves are the parent's address for the first and the
// previous one's, the window's at first, for the second). EAX is left as the last parent, or the window when there is none.
u8 *__fastcall Window_OffsetByParents(u8 *window, void *, u8 *rect);

// 0x65bcad (thiscall (parent, flags, id, left, top, right, bottom: words), pops 28; name unconfirmed): the constructor: five rectangles
// (+8, +0x14, +0x20, +0x2c, +0x38; GuiRect_Ctor), the vtable 0x705980, the flags at +0x48, the parent at +0x60, the id at +0x8c, the
// handlers +0x114 / +0x118 0x65f6d1 and most fields cleared (+0x90 / +0x94 -1, the word +0x50 0xffff). The rectangle +8 is set from the
// four words (offset by the parents when flag 0x400 is set: Window_OffsetByParents), saved to +0x2c (Window_SaveRect) and +0x14, and +0x44
// points to it; [0xfaa5e4] becomes the window. The depth word +0xe0 is the parent's + 2 (1 with none, at most 0x100); +0xe4 / +0xe6 the
// rectangle's centre from the words (sic: the parent's, or left and bottom, are stored first and overwritten). +0xa0 cleared for 0x40 bytes;
// with flag 1 the hover is updated (Window_UpdateHover); then the window is posted 0x15 (0, 0). Returns the window.
u8 *__fastcall Window_Ctor(u8 *window, void *, u8 *parent, u32 flags, u32 id, u32 left, u32 top, u32 right, u32 bottom);

// 0x6592b6 (cdecl (window); name unconfirmed): each of the globals [0xfaa5e0] (the hovered window), [0xfaa5dc], [0xfaa5e8] and the
// three from 0xfaa588 that is the window cleared. EAX is left 3 (the loop's count).
u32 __cdecl Window_ForgetGlobals(u8 *window);

// 0x65e672 (thiscall; name unconfirmed): when the window is the modal one [0xfaa57c] (msgqueue's first taker of 0x1000a-0x1000c), it and
// its pairs [0xfaa578] are cleared and 1 is in AX; else 0. Only AX is set (sic: the rest of EAX is [0xfaa57c]'s).
u32 __fastcall Window_DropModal(u8 *window);

// 0x65c2d8 (thiscall; name unconfirmed): the window and its children (from +0x5c by +0x54, each in turn) told they are going: sent 0x14
// (0, 0) (Window_SendMessage), then the byte +4 set; after the children, Window_DropModal, and [0xfaa5ec] and [0xfaa5dc] cleared when
// they are the window. EAX is left as [0xfaa5ec] as it was read.
u32 __fastcall Window_Destroying(u8 *window);

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
