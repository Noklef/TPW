// More tiny getters (bit tests, two-way selectors, counters, defaults for an empty pointer): reimplementation of
// 0x41a980, 0x56dd50, 0x516c00, 0x6c6ff0, 0x6c7000, 0x6b0600, 0x6b0610, 0x4bae30, 0x6b9da0, 0x591a60, 0x4c9ed0,
// 0x6b9260, 0x409ce0, 0x633b70, 0x633b80, 0x4f0240 and 0x67d340. Named by what they do until the classes are known.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x41a980 / 0x56dd50 (thiscall): the dword at +0 shifted right by 2 / ANDed with 0x1000000.
u32 __fastcall Obj_Get0Shr2(const u8 *object);
u32 __fastcall Obj_Get0Bit24(const u8 *object);

// 0x516c00 (cdecl): whether the word at 0x7cf4f4 is not 0.
bool __cdecl Word7cf4f4_NonZero();

// 0x6c6ff0 / 0x6c7000 (thiscall): 16 / 2 when bit 1 / bit 6 of the byte at +0x20 is set, else 8 / 1.
u32 __fastcall Obj_Bit1At20_16Or8(const u8 *object);
u32 __fastcall Obj_Bit6At20_2Or1(const u8 *object);

// 0x6b0600 / 0x6b0610 (thiscall): 4 / 14 when the dword at +8 is not 0, else 5 / 3.
u32 __fastcall Obj_NonZeroAt8_4Or5(const u8 *object);
u32 __fastcall Obj_NonZeroAt8_14Or3(const u8 *object);

// 0x4bae30 (cdecl): 0x7cc4f0 when the byte at 0x7cc53c is not 0, else 0.
u8 *__cdecl Global7cc53c_Pick7cc4f0();

// 0x6b9da0 / 0x591a60 (thiscall): the dword at +0x30 / -0x4c incremented; returns the new value.
u32 __fastcall Obj_Inc30(u8 *object);
u32 __fastcall Obj_IncBefore4c(u8 *object);

// 0x4c9ed0 (cdecl): the dword at 0x7852f8 times the one at 0x78548c (signed, low 32 bits).
s32 __cdecl Global7852f8_Times78548c();

// 0x6b9260 (thiscall): bit 1 of the dword at +0x28 when its bit 0 is set, else 0.
u32 __fastcall Obj_Flags28_Bit1IfBit0(const u8 *object);

// 0x409ce0 (stdcall; link, pops 4): advances a pointer to a node (+0 = the value, +4 = the next node) to the next node
// and returns the value of the one it left.
u32 __stdcall Link_TakeNext(u32 *link);

// 0x633b70 / 0x633b80 (thiscall): the dword at +0x14 / +0x1c, or the application instance / window when it is -1.
u32 __fastcall Obj_Get14OrInstance(const u8 *object);
u32 __fastcall Obj_Get1cOrWindow(const u8 *object);

// 0x4f0240 / 0x67d340 (thiscall): the dword at +4, or 0x6fd6c4 / 0x7089cc when it is 0.
u32 __fastcall Obj_Get4Or6fd6c4(const u8 *object);
u32 __fastcall Obj_Get4Or7089cc(const u8 *object);

}  // extern "C"
