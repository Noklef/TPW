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

// 0x642100 / 0x642b20: the constants 0xc0000 and 1000000.
u32 __cdecl Const_C0000();

// 0x631270 (a virtual that does nothing); 0x631970 (thiscall): the dword at +0x3c.
void __cdecl Stub_Ret2();

u32 __cdecl Const_1000000();

// 0x5f4ea0: returns 1. (The original runs MOVAPS XMM0,XMM0 first: the SSE probe 0x579a80 guards, which faults where the
// operating system has not enabled SSE; left out, as every Windows the port runs on has.)
u32 __fastcall ReturnOne();

// 0x564a50 / 0x5ceaa0 (cdecl), 0x5cea90 (stdcall; one argument), 0x6bab20 (stdcall; four arguments): -1.
s32 __cdecl Const_MinusOne_564a50();

s32 __cdecl Const_MinusOne_5ceaa0();

s32 __stdcall Const_MinusOne_5cea90(u32);

s32 __stdcall Const_MinusOne_6bab20(u32, u32, u32, u32);

// 0x46cfb0 (cdecl): only a debug print in the original (not reproduced).
void __cdecl Debug_Nop_46cfb0();

// 0x424ec0: returns 0x1.
u32 __cdecl Const1_424ec0();

// 0x4f6080: returns 0x7ffffffd.
u32 __cdecl Const7ffffffd_4f6080();

// 0x4ff4f0: returns 0xe.
u32 __cdecl Conste_4ff4f0();

// 0x5061a0: returns 0xf.
u32 __cdecl Constf_5061a0();

// 0x50c1f0: returns 0x1.
u32 __stdcall Const1_50c1f0(u32 v0);

// 0x569d00: returns 0x1.
u32 __stdcall Const1_569d00(u32 v0);

// 0x56a430: returns 0x1.
u32 __stdcall Const1_56a430(u32 v0, u32 v1, u32 v2);

// 0x56a440: returns 0x1.
u32 __stdcall Const1_56a440(u32 v0, u32 v1, u32 v2, u32 v3);

// 0x5cec60: returns 0.
u32 __cdecl Zero_5cec60();

// 0x5d60c0: returns 0.
u32 __cdecl Zero_5d60c0();

// 0x66f4d0: returns 0.
u32 __cdecl Zero_66f4d0();

// 0x699ca0: does nothing.
void __cdecl Stub_699ca0();

// 0x6b70d0: does nothing (pops 8 bytes).
void __stdcall Stub_Ret8_6b70d0(u32 v0, u32 v1);

// 0x6b71c0: returns 0x3.
u32 __cdecl Const3_6b71c0();

// 0x6b9bd0: returns 0x1.
u32 __stdcall Const1_6b9bd0(u32 v0, u32 v1);

// 0x6bd790: returns 0x4.
u32 __cdecl Const4_6bd790();

// 0x6c54a0: returns 0.
u32 __stdcall Zero_6c54a0(u32 v0);

// 0x6c54b0: returns 0.
u32 __stdcall Zero_6c54b0(u32 v0, u32 v1, u32 v2, u32 v3);

// 0x414670 (cdecl): the low word of the value.
u32 __cdecl Low16_414670(u32 value);

// 0x44e600 (cdecl; a, b, c, out): *out = 1; returns 1 (an init step's answer, its other arguments unused).
u32 __cdecl OutOne_44e600(u32 a, u32 b, u32 c, u32 *out);
}
