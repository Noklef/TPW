// Tiny allocating and clearing functions: node allocators, buffer clears, a list head constructor, an array allocator:
// reimplementation of 0x46cfb0, 0x47b1a0, 0x47b170, 0x5b2fd0, 0x5b1680, 0x5cd400, 0x5d3830, 0x5d3860, 0x4254e0 and 0x569c80.
// Named by what they do until the classes are known.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x46cfb0 (cdecl): only a debug print in the original (not reproduced).
void __cdecl Debug_Nop_46cfb0();

// 0x47b1a0 (thiscall): when the byte at +0 is not 0 the block at +4 is freed (the fields are left).
void __fastcall Buffer_FreeIfOwned(u8 *object);

// 0x47b170 (thiscall): the block at +4 is freed; the dwords at +4, +8 and +0xc = 0. Returns 0.
u32 __fastcall Buffer_FreeAndClear(u8 *object);

// 0x5b2fd0 (stdcall; a, b): a new block of 0x28 bytes (otherwise left as it is) with a at +4 and b at +0x24. Returns it.
u8 *__stdcall Node28_New(u32 a, u32 b);

// 0x5b1680 (stdcall; a, b): a new block of 0x18 bytes (otherwise left as it is) with a at +0 and b at +4, each the block itself when 0.
// Returns it.
u8 *__stdcall Node18_New(u32 a, u32 b);

// 0x5cd400 (thiscall): the byte at +0 = the top byte of the object pointer (sic: the original reads the byte above the saved ECX on the
// stack), a new circular head node (0x18 bytes, +0 and +4 pointing to itself) at +4, the count at +8 = 0 and its first dword at +0xc.
// Returns the object.
u8 *__fastcall ListHead_Ctor(u8 *object);

// 0x5d3830 (thiscall; a, b, c, pops 12): a at +0, b at +4, c at +8, a new block of a * b * c bytes at +0xc. Returns the object.
u8 *__fastcall Array3D_Init(u8 *object, void *, u32 a, u32 b, u32 c);

// 0x5d3860 (thiscall): the dwords at +0, +4 and +8 = 0; the block at +0xc, when there is one, is freed and the pointer cleared.
void __fastcall Array3D_Free(u8 *object);

// 0x4254e0 (cdecl; object, from, to): Obj_RemoveFromRect of the object at +0x24 (from, to); then bit 14 of the dword at +0 is cleared.
// Returns 1.
u32 __cdecl Obj_RemoveRectAndClear14(u8 *object, u32 from, u32 to);

// 0x569c80 (thiscall; flags, pops 4): Vtbl701260_Set_565230; when bit 0 of the flags is set the block 0x2c bytes before the object is freed.
// Returns that address (the object - 0x2c).
u8 *__fastcall Deleter_Minus2c_569c80(u8 *object, void *, u32 flags);

}  // extern "C"
