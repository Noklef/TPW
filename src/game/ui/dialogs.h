// The game's dialog windows (each a global window pointer) sent their messages: Window_SendMessage wrappers. Reimplementation of 0x4b87f0,
// 0x4b8810 and 0x4b8830.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x4b87f0 / 0x4b8810 / 0x4b8830 (cdecl; names unconfirmed): Window_SendMessage of the window [0x7cc47c] with message 4 (0, 0), message 6
// (1, 0) and message 6 (0, 0); its result returned.
s32 __cdecl Dialog7cc47c_Message4();
s32 __cdecl Dialog7cc47c_Message6On();
s32 __cdecl Dialog7cc47c_Message6Off();

// 0x4b8e80 / 0x4b8ec0 (cdecl; names unconfirmed): Window_SendMessage of the window [0x7cc4b4] with message 4 (0, 0) and message 6 (0, 0).
s32 __cdecl Dialog7cc4b4_Message4();
s32 __cdecl Dialog7cc4b4_Message6Off();

// 0x4b9c00 / 0x4b9c20 (cdecl; names unconfirmed): Window_SendMessage of the window [0x7cc4c4] with message 4 (0, 0) and message 6 (1, 0).
s32 __cdecl Dialog7cc4c4_Message4();
s32 __cdecl Dialog7cc4c4_Message6On();

// 0x4c5780 / 0x4c57a0 / 0x4c57c0 (cdecl; names unconfirmed): Window_SendMessage of the window [0x7cdb0c] with message 4 (0, 0), message 6
// (1, 0) and message 6 (0, 0).
s32 __cdecl Dialog7cdb0c_Message4();
s32 __cdecl Dialog7cdb0c_Message6On();
s32 __cdecl Dialog7cdb0c_Message6Off();

// 0x4c57e0 / 0x4c5800 (cdecl; names unconfirmed): the same with message 6 (1, 0) / (0, 0) to its child 0x16007 (Window_FindChild; none:
// the send's -1).
s32 __cdecl Dialog7cdb0c_Child16007Message6On();
s32 __cdecl Dialog7cdb0c_Child16007Message6Off();

// 0x48d990 (cdecl; name unconfirmed): when the window [0x7ca098] is not 0, Window_SendMessage of it with message 4 (0, 0); then the dword
// 0x7ca090 = 0. Returns the send's result (0 without a window).
s32 __cdecl Dialog7ca098_Message4IfUp();

// 0x4c25e0 (cdecl; name unconfirmed): when the window [0x7cca94] is not 0, Window_SendMessage of it with message 4 (0, 0) (its result; 0
// without a window).
s32 __cdecl Dialog7cca94_Message4IfUp();

// 0x48cf70 / 0x492e60 (thiscall; names unconfirmed): Window_SendMessage with message 4 (0, 0) of the window at +0x10 of the object (0x492e60
// only when it is not 0; 0 then).
s32 __fastcall WindowAt10_Message4(u8 *object);
s32 __fastcall WindowAt10_Message4IfAny(u8 *object);

// 0x48c8b0 (thiscall (flags), pops 4; name unconfirmed): the scalar deleting destructor: WindowAt10_Message4IfAny, then the object freed
// (GameHeap_Free) for flags bit 0; returns the object.
u8 *__fastcall WindowAt10_ScalarDtor(u8 *object, void *, u32 flags);

// 0x48ee20 (cdecl; name unconfirmed): message 5 (0, 0) sent (Window_SendMessage) to the window (+0) of each of the ten entries of 0x208 bytes
// from 0x7c26b0 whose word +4 is not 0.
void __cdecl Windows7c26b0_Message5();

// 0x4bcef0 (cdecl; name unconfirmed): with `same` = (the dword 0x7cc698 == the window [0x7cc8bc]), message 6 (same, 0) to [0x7cc8bc] and
// message 6 (!same, 0) to [0x7cc8c0] (Window_SendMessage; the second's result returned); then 0x7cc698 = 0.
s32 __cdecl Windows7cc8bc_Message6Pair();
}
