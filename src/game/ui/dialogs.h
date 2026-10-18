// The game's dialog windows (each a global window pointer) sent their messages: Window_SendMessage wrappers. Reimplementation of 0x4b87f0,
// 0x4b8810 and 0x4b8830 and others (0x4bcec0 chooses the window 0x7cc698).
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

// 0x4b8ee0 (cdecl (window, on: a byte); the 0x7cc4b4 dialog's, whose handler calls it with (0, 0) as it goes; name unconfirmed): on: the
// window's inner rectangle (Window_GetRect, through a GUI rectangle on the stack) gives its vertical centre, top + (bottom - top) / 2
// (an arithmetic shift), and unless the particle system [0x7cc4bc] is already set it becomes Particle_Spawn(0x61, 50000, 0, centre *
// 75000 / 1536 (32-bit, wrapping; the quotient toward 0)). Off: the system ended (ParticleSystem_End) and [0x7cc4bc] cleared when set.
void __cdecl Dialog7cc4b4_SetEffect(u8 *window, u32 on);

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

// 0x492e00 (cdecl (window, message, a, b); the handler of such an object's window, also called from 0x48bb5f and 0x48c0be; name unconfirmed): on
// message 0x14 the object (Window_DwordA0 0) forgets its window (+0x10 = 0); then Window_CallDefault with the message, its result returned.
s32 __cdecl WindowAt10_Handler(u8 *window, u32 message, u32 a, u32 b);

// 0x48c8b0 (thiscall (flags), pops 4; name unconfirmed): the scalar deleting destructor: WindowAt10_Message4IfAny, then the object freed
// (GameHeap_Free) for flags bit 0; returns the object.
u8 *__fastcall WindowAt10_ScalarDtor(u8 *object, void *, u32 flags);

// 0x48ee20 (cdecl; name unconfirmed): message 5 (0, 0) sent (Window_SendMessage) to the window (+0) of each of the ten entries of 0x208 bytes
// from 0x7c26b0 whose word +4 is not 0.
void __cdecl Windows7c26b0_Message5();

// 0x4bcef0 (cdecl; name unconfirmed): with `same` = (the dword 0x7cc698 == the window [0x7cc8bc]), message 6 (same, 0) to [0x7cc8bc] and
// message 6 (!same, 0) to [0x7cc8c0] (Window_SendMessage; the second's result returned); then 0x7cc698 = 0.
s32 __cdecl Windows7cc8bc_Message6Pair();

// 0x4bcec0 (cdecl): the window 0x7cc698 (that Windows7cc8bc_Message6Pair compares) = [0x7cc8b8] if set, else [0x7cc8bc] if the byte
// 0x7cc8b4 is set, else [0x7cc8c0]. Returns the value stored in the first two cases, else 0.
u32 __cdecl Global7cc698_Choose();

// 0x4a2590 (cdecl (value); name unconfirmed): with the dialog [0x7cb2b8] and a value not 0, its child 0x21 (Window_FindChild): 0 when that
// child's dword +0xa0 slot 2 (Window_DwordA0) is set; else message 0xb (0, 0) sent to it (Window_SendMessage) and message 0xa (value, 0)
// posted (Window_PostMessage). Returns the value otherwise (no dialog, value 0 or no such child included).
u32 __cdecl Dialog7cb2b8_Child21Post(u32 value);
}
