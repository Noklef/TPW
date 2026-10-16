// Tiny functions reached through vtables and from many call sites: reimplementation of 0x5b96a0,
// 0x5b96b0, 0x5c5520, 0x5c5550, 0x5c55b0, 0x5cec70, 0x5d1410, 0x5d1750, 0x5da3d0,
// 0x5e7310 and 0x5e7720. Whose they are isn't known yet, so they take their names from what they do.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x5b96a0 / 0x5b96b0 (thiscall): the dword at +0x3e4 / +0x3a4.
u32 __fastcall Obj_Get3e4(const u8 *object);
u32 __fastcall Obj_Get3a4(const u8 *object);

// 0x5c5520 (thiscall): the object itself.
u8 *__fastcall Obj_Self(u8 *object);

// 0x5c5550 / 0x5c55b0 (thiscall): the address of the part at +0x14 / +0x3c.
u8 *__fastcall Obj_Part14(u8 *object);
u8 *__fastcall Obj_Part3c(u8 *object);

// 0x5cec70 / 0x5da3d0 (thiscall virtuals that do nothing; the callee pops 4 / 0xc bytes).
void __stdcall Stub_Ret4(u32 a);
void __stdcall Stub_Ret12(u32 a, u32 b, u32 c);

// 0x5d1410 (thiscall): 1.
u32 __fastcall Stub_One(void *object);

// 0x5d1750 (cdecl): nothing.
void __cdecl Stub_Ret();

// 0x5e7720 (thiscall; dword, byte): a pair built from a dword at +0 and a byte at +4, copied from
// the two pointers, the dword stored before the byte is read. Returns the pair.
u8 *__fastcall Pair_Assign(u8 *pair, void *, const u32 *dword, const u8 *byte);

// 0x5e7310 (thiscall): +0x38 = the dword at +0xc; returns that dword with its low byte set to 1 (sic).
u32 __fastcall Obj_CopyCToMark(u8 *object);
}
