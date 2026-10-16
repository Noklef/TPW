// Small functions: a table clear, post-increments of two map iterators, constructor wrappers, a list add that records the item, a class constructor with its static
// initialiser and three copies of game strings: reimplementation of 0x56a750, 0x4f0370, 0x4f0460, 0x4f0880, 0x4f08a0, 0x421680, 0x5f9f80, 0x5f9f70, 0x5ed490, 0x5ed4d0
// and 0x5ed560. Named by what they do until the callers are known.
#pragma once

#include "../../../re/types.h"
#include "string.h"

extern "C" {

// 0x56a750 (cdecl): the 64 pairs of dwords from 0x879ad0 and the dword at 0x879cd0 are cleared.
void __cdecl Table879ad0_Clear();

// 0x4f0370 / 0x4f0460 (thiscall on an iterator; out, int, pops 8): *out = the old node, the iterator moved on (StdTree_IncC / StdTree_IncE). Returns out.
u8 **__fastcall StdTreeC_PostInc(u8 **it, void *, u8 **out, s32 unused);
u8 **__fastcall StdTreeE_PostInc(u8 **it, void *, u8 **out, s32 unused);

// 0x4f0880 / 0x4f08a0 (stdcall; the object and 3 / 7 more dwords that are not used): Obj_Init4f1520 with -1 on the object. Returns it.
u8 *__stdcall Obj_InitMinus1A(u8 *object, u32, u32, u32);
u8 *__stdcall Obj_InitMinus1B(u8 *object, u32, u32, u32, u32, u32, u32, u32);

// 0x421680 (thiscall; item, pops 4): +8 = item, then ObserverList0_Add of the object with it. Returns what that returns.
u32 __fastcall Obj_SetAt8AndAdd(u8 *object, void *, u8 *item);

// 0x5f9f80 (thiscall): the vtable pointer 0x7035d8 at +4, the byte at +0 and the words at +8, +0xa, +0xc cleared. Returns the object. 0x5f9f70 (cdecl): that for the global
// object at 0xfa78e8 (its static initialiser).
u8 *__fastcall Class7035d8_Ctor(u8 *object);
u8 *__cdecl Class7035d8_InitGlobal();

// 0x5ed490 (stdcall; first, last, dest, pops 12): a copy (String_CtorCopy) of each string of the array from first up to last (0x14 bytes each) is built in the array at dest
// (a null dest only counts). Returns the end of dest. 0x5ed4d0 (stdcall; dest, count, from): the same string copied into count strings from dest. 0x5ed560 (cdecl;
// dest, from): one copy built at dest when it is not null.
String *__stdcall Strings_CopyRange(const String *first, const String *last, String *dest);
void __stdcall Strings_FillCopy(String *dest, u32 count, const String *from);
void __cdecl String_PlacementCopy(String *dest, const String *from);

}  // extern "C"
