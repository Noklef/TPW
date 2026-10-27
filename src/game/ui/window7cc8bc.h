// The window [0x7cc8bc]'s name list (its child 0xc3123's child 0xc3124, a 0x7059f0 list) kept by name in either of its modes (the dword
// [0x7cc8d0] 0 or 1), and the handler of the windows [0x7cc8c4] / [0x7cc8c8]. Reimplementation of 0x4beb60-0x4bedd1 and the stdcall
// wrappers 0x481820-0x4818b2. The names are unconfirmed (the callers are 0x4e7000-0x4ee800's).
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

}  // extern "C"
