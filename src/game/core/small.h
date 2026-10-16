// More tiny functions (vtable pointer setters and constructors, getters, a divide, a dword copy):
// reimplementation of 0x5b3210, 0x617920, 0x617ad0, 0x6185f0, 0x631270, 0x631970, 0x6322e0, 0x6338b0,
// 0x633a00, 0x6339a0, 0x6339e0, 0x633b60, 0x6344c0, 0x6404d0, 0x6428b0, 0x642100 and 0x642b20,
// and the constructor 0x631280. Named by what they do until the
// classes are known.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x5b3210 (cdecl; dest, src): *(u16 *)dest = *(u16 *)src, unless dest is 0.
void __cdecl Word_CopyTo(u16 *dest, const u16 *src);

// 0x617920 / 0x617ad0 / 0x6338b0 (thiscall): the vtable pointer +0 = 0x703748 / 0x703770 / 0x703770;
// the last also sets +4 = 1 and returns the object.
void __fastcall Vtbl703748_Set(u8 *object);
void __fastcall Vtbl703770_Set(u8 *object);
u8 *__fastcall Class703770_Ctor(u8 *object);

// 0x6185f0 (thiscall): the dword at +0x64, plus 0x10.
u32 __fastcall Obj_Get64Plus10(const u8 *object);

// 0x631270 (a virtual that does nothing); 0x631970 (thiscall): the dword at +0x3c.
void __cdecl Stub_Ret2();
u32 __fastcall Obj_Get3c(const u8 *object);

// 0x6322e0 (cdecl; first, last, dest): the dwords from first up to last copied to dest, forwards;
// returns the end of what was written (dest when none).
u32 *__cdecl Dwords_Copy(const u32 *first, const u32 *last, u32 *dest);

// 0x633a00 (thiscall): vtable 0x703c80, +4 = +8 = 5, +0xc = 0, +0x10 = 1, +0x14 = +0x18 = +0x1c = -1.
u8 *__fastcall Class703c80_Ctor(u8 *object);

// 0x6339e0 (thiscall; source, pops 4): +0x14 = 0, the word +0xe = the source's word at +0xc,
// +0x10 = the source's dword at +0x10, the byte +0xc = 3. Returns that dword.
u32 __fastcall Obj_Take(u8 *object, void *, const u8 *source);

// 0x6339a0 (thiscall; source, pops 4): +0x14 = the source's dword at +0x20; the byte +0xd = its low
// byte at +0x10; the word +0xe = the new +0x14's bit 3 (0 or 8); the byte +0xc = 1; +0x10 = the
// source's words at +0x14 and +0x18 as low and high halves. Returns the source.
const u8 *__fastcall Obj_TakeWords(u8 *object, void *, const u8 *source);

// 0x6344c0 (thiscall; source, pops 4): 4 dwords at +0x34 from the source (nothing for a null
// source). Returns the source's fourth dword, or 0 for a null source.
u32 __fastcall Obj_Copy4At34(u8 *object, void *, const u32 *source);

// 0x6404d0 (cdecl; code, length): the low `length` bits of code in reverse order (zlib's
// bi_reverse), at least one bit being taken.
u32 __cdecl Bits_Reverse(u32 code, s32 length);

// 0x631280 (cdecl; object): the object's function pointer table (0x6312d0, 0x631340, 0x631300,
// 0x631380, 0x631440 at +0 to +0x10) and fields (+0x14, +0x68, +0x6c, +0x78-+0x80 = 0, +0x70 =
// 0x7039d0, +0x74 = 0x7b). Returns the object.
u8 *__cdecl Codec631280_Init(u8 *object);

// 0x633b60 (thiscall; value): +0x10 = value; returns it.
u32 __fastcall Obj_Set10(u8 *object, void *, u32 value);

// 0x6428b0 (cdecl; a, b): (a + b - 1) / b, a signed division.
s32 __cdecl DivCeil(s32 a, s32 b);

// 0x642100 / 0x642b20: the constants 0xc0000 and 1000000.
u32 __cdecl Const_C0000();
u32 __cdecl Const_1000000();
}
