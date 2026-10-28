// The window [0x7cc8bc]'s name list (its child 0xc3123's child 0xc3124, a 0x7059f0 list) kept by name in either of its modes (the dword
// [0x7cc8d0] 0 or 1), the handler of the windows [0x7cc8c4] / [0x7cc8c8], and their creation. Reimplementation of 0x4beb60-0x4bedd1,
// 0x4bf460, 0x4bd090 and the stdcall wrappers 0x481820-0x4818b2. The names are unconfirmed (the callers are 0x4e7000-0x4ee800's).
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x4beb60 / 0x4bec60 (cdecl (text, data)): with the window [0x7cc8bc] set and the mode [0x7cc8d0] 0 / 1, an item added to its list
// (Window7059f0_AddItem(texts, data, parent -1, refresh 1)): the texts a stack slot holding text, so only the first column's is the
// caller's (a second column's would be the return address, sic). The list's children are not checked.
void __cdecl Window7cc8bc_ListAdd(const u16 *text, u32 data);
void __cdecl Window7cc8bc_ListAdd1(const u16 *text, u32 data);

// 0x4bebb0 / 0x4becb0 (cdecl (text)): with the window and the mode as above, the list's item whose first column is text
// (Window7059f0_FindText(text, 1)) removed (Window7059f0_RemoveItem) when there is one.
void __cdecl Window7cc8bc_ListRemove(const u16 *text);
void __cdecl Window7cc8bc_ListRemove1(const u16 *text);

// 0x4bec00 / 0x4bed00 (cdecl (text, data)): with the window and the mode as above, the list's item whose first column is text given the
// data (Window7059f0_SetNthItemData) and the rows refreshed (Window7059f0_RefreshRows) when there is one.
void __cdecl Window7cc8bc_ListSetData(const u16 *text, u32 data);
void __cdecl Window7cc8bc_ListSetData1(const u16 *text, u32 data);

// 0x481820 / 0x481840 / 0x481850 / 0x481870 / 0x481890 / 0x4818a0 (stdcall): the six above, in that order (0x4beb60, 0x4bebb0, 0x4bec00,
// 0x4bec60, 0x4becb0, 0x4bed00).
void __stdcall Window7cc8bc_ListAddStd(const u16 *text, u32 data);
void __stdcall Window7cc8bc_ListRemoveStd(const u16 *text);
void __stdcall Window7cc8bc_ListSetDataStd(const u16 *text, u32 data);
void __stdcall Window7cc8bc_ListAdd1Std(const u16 *text, u32 data);
void __stdcall Window7cc8bc_ListRemove1Std(const u16 *text);
void __stdcall Window7cc8bc_ListSetData1Std(const u16 *text, u32 data);

// 0x4bed60 (cdecl (window, message, a, b); the handler 0x4bf650 / 0x4bf699 give): message 0x14: the dword [0x7cc8c4] cleared when it is
// the window, else [0x7cc8c8] cleared; 0x10006: the window [0x7cc8cc] hidden (Window_SetShown(0)) and unlinked (Window_Unlink), the
// focus of [0x7cc8bc] dropped (Window_SetFocus(0)); none of them checked. Then the default handler's result (Window_CallDefault).
s32 __cdecl Windows7cc8c4_Handler(u8 *window, u32 message, u32 a, u32 b);

// 0x4bf460 (cdecl; name invented): the mode [0x7cc8d0] = 0; the window [0x7cc8bc] built on the GUI root [0xfaa5d8] from the script
// 0x758ae8 with the handler 0x4bd4a0 (WindowScript_Build). When made: its child 0xc3123 the handler 0x4be290; that one's list 0xc3124
// given rows (Window7059f0_BuildRows: List758f04_NewRowA, List758f04_InitRowJump), column 0's bottom 0, the registered "hilight" as its
// highlight, the handler 0x4be820, and its radio group 0xc3125 on 0xc3126; the list 0xc312b rows (List758f04_NewRowB,
// List758f04_InitRowJumpB) and column 0's bottom 0; that one's edit box 0xc312c the handler 0x4be3a0, a 0x6ff4d0 text part (0x1a4
// bytes, none when the allocation fails) as its impl, the font [0x758f08], the buffer 0x7cc6a8 of 0xff, "hilight" as its caret and
// the text 0x78a374 (slot 9); the byte [0x7cc8b4] = 1; the window's inner rectangle got (Window_GetRect, unused) and [0x7a1c54] /
// [0x7a1c58] / [0x74d108] / [0x74d104] = [0x7a192c] / [0x7a194c] / [0x7a162c] / [0x7a0c9c]. None of the children is checked. Then,
// either way, the windows [0x7cc8c4] / [0x7cc8c8] from the scripts 0x758cd8 / 0x758e18 with the handler 0x4bed60, hidden, unlinked and
// their children 0x1789 / 0x3003 given the handler 0x4bede0 (not checked); [0x7cc8cc] = 0; [0x7cc8c0] a new kind 1 window under the
// root (Window_CreateChild: flags 0, id 0xfff04d2a, 0, 0, 0x7ff, 0x5ff) with the handler 0x4bd4a0; the help background placed
// (HelpBg_Place(-1, 10)); [0x7cc8e4] = 0 and the byte [0x7cc8d4] = 0.
void __cdecl Window7cc8bc_Create();

// 0x4bd090 (cdecl; name invented): [0x7cc8b8] = [0x7cc8bc] with the byte [0x7cc8b4] set, else [0x7cc8c0]; the inner rectangle of
// [0x7cc8c0] got (Window_GetRect, unused); message 6 (0, 0) to [0x7cc8bc] and [0x7cc8c0]; the floats [0x7a1c70] / [0x7a1c78] in tenths
// packed as Camera_PackCellFractions does into 0x7cc690 / 0x7cc694 (inlined); the chat [0x7cddec] (made when there is none: 0xf0c
// bytes, OnlineChat_Ctor) arrives at (the words [0x7cc690], [0x7cc694]) (ChatAvatar_Arrive: x's high word EAX's, the last __ftol's or
// the new chat's, sic) and, with [0x790ab0] bit 2, says its position (ChatAvatar_SayPosition, the chat made again when none); the byte
// [0x7cc8d4] = 1, [0x7cc8ac] = [0x790ab0], [0x7cc8d8] = 5.
void __cdecl Window7cc8bc_EnterChat();

// 0x4bd260 (cdecl; name invented): nothing with [0x790ab0] bit 10; else message 6 (1 for the window [0x7cc8b8] was, else 0; 0) to
// [0x7cc8bc] and the other to [0x7cc8c0], [0x7cc8b8] = 0, the cursor 0 on each of them that is set (Window_SetCursor); the floats
// [0x790aa0] / [0x790aa8] in tenths packed into 0x7cc690 / 0x7cc694 as Window7cc8bc_EnterChat packs its; the chat (OnlineChat_Instance)
// enters at (the words [0x7cc690], [0x7cc694]) (ChatAvatar_Enter: y's high word the last __ftol's, sic); the byte [0x7cc8d4] = 0, and
// the floats [0x790ad0] / [0x790ad8] = (fraction / 256 + whole) * 10 of the packed bytes.
void __cdecl Window7cc8bc_LeaveChat();

// 0x4bf370 (cdecl (unused, window); name invented; nothing in the image calls it or holds its address): the window [0x7cc8cc] (read
// first) becomes the last child of the list 0xc3124 of the child 0xc3123 of [0x7cc8bc] (Window_FindChild twice, unchecked;
// Window_AddChild(1, 1)). The window's inner rectangle (Window_GetRect(.., 1) into a GuiRect on the stack) is taken, and the window is
// moved (Window_Offset) by the mouse position (Gui_GetMousePos) less that rectangle's left / top. The original reads them as dwords
// (the mouse words' upper halves are stack leftovers); only the low words count. Then [0x7cc8cc] (read again each time) gets its depth
// word +0xe0 + 4 (Window_SetDepth; the pushed word's upper half is the rectangle's right, which the setter ignores), is shown
// (Window_SetShown(1)) and takes the focus (Window_SetFocus(0)).
void __cdecl Window7cc8cc_PlaceAtMouse(u32 unused, u8 *window);

}  // extern "C"
