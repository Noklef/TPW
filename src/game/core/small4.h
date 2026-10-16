// More tiny functions: vtable stores, constants, small getters and thin wrappers over ported functions: reimplementation of
// 0x4056f0, 0x504740, 0x429b80, 0x47a9a0, 0x5cea30, 0x4f04c0, 0x564a50, 0x5ceaa0, 0x5cea90, 0x6bab20, 0x5d5db0, 0x5077f0 and 0x486780.
// Named by what they do until the classes are known.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x4056f0 (thiscall): the address 4 bytes into the object.
u8 *__fastcall Obj_AddrOf4(u8 *object);

// 0x504740 / 0x429b80 (thiscall): the vtable pointer at +0 / +0x60 = 0x6fd800 / 0x6fdce8. Undefined return.
void __fastcall Vtbl6fd800_Set(u8 *object);
void __fastcall Vtbl6fdce8_At60_Set(u8 *object);

// 0x47a9a0 (thiscall): the dword at +0xc, or 0x6fd6c4 when it is 0.
u32 __fastcall Obj_GetCOr6fd6c4(const u8 *object);

// 0x5cea30 (thiscall): like VirtualBase_SetVtbl7099dc, with 0x70994c (8 bytes before the object).
u32 __fastcall VirtualBase_SetVtbl70994c(u8 *object);

// 0x4f04c0 (thiscall; index, pops 4): the address of the index-th element of 36 bytes of the array at +4.
u8 *__fastcall Array36_At(const u8 *object, void *, u32 index);

// 0x564a50 / 0x5ceaa0 (cdecl), 0x5cea90 (stdcall; one argument), 0x6bab20 (stdcall; four arguments): -1.
s32 __cdecl Const_MinusOne_564a50();
s32 __cdecl Const_MinusOne_5ceaa0();
s32 __stdcall Const_MinusOne_5cea90(u32);
s32 __stdcall Const_MinusOne_6bab20(u32, u32, u32, u32);

// 0x5d5db0 (cdecl): the dword at 0x786b80 = 1 and the dword at +0x14 of what the dword at 0xf82884 points to = 3. Returns that
// pointer.
u32 __cdecl Global786b80_SetAndMode3();

// 0x5077f0 (thiscall): Staff_WageFor of the dword at +0 and the byte at +8.
s32 __fastcall Staff_WageForRecord(const u8 *record);

// 0x486780 (cdecl; wide text): when the dword at 0x78ba80 is set, the text is converted to bytes (Narrow_FromWide, room 0x3fff) at
// 0x7b5378. Returns 0x7b5378.
u32 __cdecl Text_NarrowToGlobalBuffer(const u16 *text);

}  // extern "C"
