// A few small functions that belong to no known unit yet: reimplementation of 0x402ef0, 0x4032b0, 0x40f300, 0x458910, 0x458940, 0x4a9350, 0x4b4670, 0x409260, 0x45be80, 0x5f4490, 0x633dc0, 0x6486e0, 0x458310, 0x4583d0, 0x4bcec0, 0x486610, 0x4f36c0, 0x5c9040, 0x5d03b0, 0x5dd3d0, 0x5ddac0, 0x5dd810, 0x5e2750, 0x633120, 0x5d1440, 0x52fc30, 0x56b340, 0x4d09a0, 0x52fbf0, 0x424ed0, 0x47f940, 0x4922d0, 0x4d4170, 0x4d9600, 0x4e01f0,
// 0x4e1c60, 0x4f00f0, 0x4f6020, 0x4f6820, 0x4f6850.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x40f300 (thiscall; value, pops 4): +4 = 0, vtable 0x6fd7b8, Fn_59b550 on the part at +8 with the value,
// then vtable 0x6fd7b0. Returns the object.
u8 *__fastcall Obj_CtorPart8(u8 *object, void *, u32 value);

// 0x458910 / 0x458940 (cdecl; pointer to the object): flag 0x10000000 of the part the object's vtable
// (+8) reaches, at +0x50, cleared / set, the scene flags (0x8bd508) touched first.
void __cdecl MeshRef_ClearFlag10000000(u8 *const *ref);
void __cdecl MeshRef_SetFlag10000000(u8 *const *ref);

// 0x4a9350 (cdecl): the counter at 0x7cc150 stepped down if above 0; 1 when it is 0 and 0x7cc190 is
// set, else 0.
u32 __cdecl Countdown7cc150_Step();

// 0x4b4670 (thiscall): when the script slot at +0x24 is set it is killed (SpriteScript_Kill) and
// +0x24 = 0, +0x28 = +0x2c = -1; returns -1 for that, else 0.
u32 __fastcall Obj_KillSlot24(u8 *object);

// 0x4bcec0 (cdecl): 0x7cc698 = 0x7cc8b8 if set, else 0x7cc8bc if the byte 0x7cc8b4 is set, else
// 0x7cc8c0. Returns the value stored in the first two cases, else 0.
u32 __cdecl Global7cc698_Choose();

// 0x4d09a0 (thiscall): over the 8 entries of 0x20 bytes from +0x20 (size = (+0 - +0xc) * +4 when +8
// is set, else 0), the index of the smallest non-zero size (the first of equals), or -1 for none.
s32 __fastcall Obj_SmallestEntry(const u8 *object);

// 0x458310 (thiscall; mostDerived, pops 4): a class with a virtual base. When most derived: vtable
// 0x6fe570 and the part at +0x60 gets vtable 0x6fdce8. Always: the virtual base (at the vtable's +4)
// gets vtable 0x6fdcd8, the dword before it = its offset - 0x60, and +0x50 = 0. Returns the object.
u8 *__fastcall Obj_CtorVirtual(u8 *object, void *, u32 mostDerived);

// 0x4583d0 (thiscall; a, b, c, pops 12): the scene flags touched (SceneFlags_Touch on 0x8bd508), then
// +0x2c = a, +0x30 = b, +0x34 = c. Returns a.
u32 __fastcall Obj_SetVectorAt2c(u8 *object, void *, u32 a, u32 b, u32 c);


// 0x4f36c0 (thiscall): the number of 36-byte elements between the pointers at +4 and +8 (a signed
// division); 0 when +4 is null.
s32 __fastcall Vector36_Size(const u8 *vector);

// 0x52fc30 (cdecl): the pair table at 0x81b740 (count 0x820a8c) copied to 0x818d98; then 0x820a90 = the
// count and 0x820a94 = 0. Returns the count when above 0, else 0.
s32 __cdecl PairTable_Commit();

// 0x409260 (thiscall): Obj_AdvanceUnless30 on the object at 0x785970; 0x7477e4 = bit 3 of the flags
// 0x7a1a14; and when the dword at +0x18 is set bit 1 of the flags is set. Returns the flags then, else 0.
u32 __fastcall Flags7a1a14_Update(const u8 *object);

// 0x45be80 (cdecl; object, from, to): the part at the object's +0x1c (a virtual base: the pointer plus the
// offset at +4 of its vtable; 0 for none) is removed from the renderer's grid ([0x87a6d0]) over the
// rectangle from.hi, from.lo to to.hi, to.lo (the positions' high and low words as x and y
// of the corners: Renderer_RemoveInRect); then bit 0 of the object's first dword is cleared. Returns that dword.
u32 __cdecl Obj_RemoveFromRect(u8 *object, u32 from, u32 to);

// 0x486610 (cdecl): when the object at [0x7c24d8] has its +0xc set the sprite script slot [0x7c24a0] is
// made a copy of that script slot (SpriteScript_CopyFrom).
void __cdecl Global7c24d8_CopyScript();

// 0x5d1440 (stdcall; _, from, to, limit, pops 16): to - from, at most limit (unsigned).
u32 __stdcall Span_Limited(u32 unused, u32 from, u32 to, u32 limit);

// 0x5dd810 (stdcall; object, pops 4): 1 when the signed dword at +0x40 is above 0, else 0.
u32 __stdcall Obj_Positive40(const u8 *object);

// 0x5e2750 (thiscall; value, pops 4): with a value, +0xc = value and +0x14 = 0.
void __fastcall Obj_SetOrKeepC(u8 *object, void *, u32 value);

// 0x633120 (thiscall): Class703770_Ctor, then vtable 0x703c18 and +8 = +0xc = 0. Returns the object.
u8 *__fastcall Class703c18_Ctor(u8 *object);

// 0x5c9040 (thiscall): Init_5d36c0, vtable 0x70295c and the four dwords from +8 zeroed. Returns the object.
u8 *__fastcall Class70295c_Ctor(u8 *object);

// 0x5d03b0 (thiscall; other, pops 4): the dwords at +4 and +8 swapped with the other object's.
void __fastcall Obj_SwapPair(u8 *object, void *, u8 *other);

// 0x5dd3d0 (thiscall): the three dwords at +0x10 take those at +0x28 when +0x58 is set, else those at +0x1c.
void __fastcall Obj_ChooseVector(u8 *object);

// 0x5ddac0 (stdcall; object, pops 4): 1 when the dword at +0x60 is 0 and the signed +0x40 is below +0x64.
u32 __stdcall Obj_Pending(const u8 *object);

// 0x5f4490 (thiscall; value, pops 4): when the dword at +0x2c differs from the value it is set, and the object
// at +0x28 (through Fn_56b620, which returns it unchanged) has bit 0 of its dword at +0x10 cleared for a
// non-zero value, set for 0. Returns that object, or the value when nothing changed.
u32 __fastcall Obj_SetShown(u8 *object, void *, u32 value);

// 0x633dc0 (thiscall): Class703770_Ctor, then +8, +0x14, +0x18, +0x1c, +0x20 = 0, +0x10 = 0x703744 and the
// vtables 0x703ca0 (+0) and 0x703c98 (+0xc). Returns the object.
u8 *__fastcall Class703ca0_Ctor(u8 *object);

// 0x6486e0 (cdecl; records, count): over the 0x20-byte records, the first with the greatest signed dword at
// +0x1c among those where that and the dword at +0x18 are above 0; 0 for none.
u8 *__cdecl Records32_MaxWeight(u8 *records, s32 count);

// 0x52fbf0 (cdecl; a, b): appends the pair to the table of 8-byte entries at 0x81b740 (count at
// 0x820a8c), unless the count is above 0x400 (signed): 0 then, else 1.
u32 __cdecl PairTable_Add(u32 a, u32 b);

// 0x402ef0 (thiscall): when Obj_Get30 is 0, +0x3c += +0x40. Returns +0x30 if set, else +0x40.
u32 __fastcall Obj_AdvanceUnless30(u8 *object);

// 0x4032b0 (thiscall): Init_402f30, then +0x20 = 0 and +0x24 = 0x3ff00000 (the double 1.0 at +0x20).
u8 *__fastcall Obj_CtorOne(u8 *object);

// 0x47f940 (thiscall; value, pops 4): Fn_657d30 on the object, then vtable +0 = 0x6ff058 and +0x14 = value.
u8 *__fastcall Obj_CtorValue(u8 *object, void *, u32 value);

// 0x4d4170 (thiscall): the script slot at +0 killed (SpriteScript_Kill) and +0 = 0.
void __fastcall Obj_KillSlot(u8 *object);

// 0x4d9600 / 0x4e01f0 (thiscall; word, pops 4): the word at +0x30 / +0x5e = word and the dword +0x34 /
// +0x60 = the dword at +0x1da70c of the object at [0x80239c] (the world); returns that dword.
u32 __fastcall Obj_StampAt30(u8 *object, void *, u32 word);
u32 __fastcall Obj_StampAt5e(u8 *object, void *, u32 word);

// 0x424ed0 (cdecl; pointer): bit 0 of the dword cleared; returns 1.
u32 __cdecl Dword_ClearBit0(u32 *p);

// 0x4922d0 (thiscall): +0x18 = 0 and the word +0xc = 0; returns 0.
u32 __fastcall Obj_Clear18_0c(u8 *object);

// 0x4e1c60 (thiscall): the dword at +0x180 minus the one at +0x184.
u32 __fastcall Obj_Diff180_184(const u8 *object);

// 0x4f00f0 (thiscall on a sub-object; the vtable pointer at this - 4 reaches its virtual base): the
// dword at base offset - 4 + this (base offset = the vtable's +4) = 0x7099ec.
void __fastcall Vtbl7099ec_SetVirtual(u8 *sub);

// 0x4f6020 (thiscall): the wide streambuf's pointer slots pointed at its own fields: +0xc -> +4, +0x10 ->
// +8, +0x1c -> +0x14, +0x20 -> +0x18, +0x2c -> +0x24, +0x30 -> +0x28; the six fields they name are
// zeroed. Returns the address of the last (+0x24).
u32 __fastcall Wstreambuf_InitPointers(u8 *object);

// 0x4f6820 / 0x4f6850 (thiscall; the std::basic_string streambuf's setg and setp over wide characters,
// pops 12 / 8): the pointers behind +0xc, +0x1c, +0x2c (first, next, count) / +0x10, +0x20, +0x30
// get first, next and (last - next) / 2 (setg: first, next, last; setp: first, last, with next =
// first). Returns the count pointer.
u32 *__fastcall Wstreambuf_Setg(u8 *object, void *, u32 first, u32 next, u32 last);
u32 *__fastcall Wstreambuf_Setp(u8 *object, void *, u32 first, u32 last);

u8 *__fastcall Class703770_Ctor(u8 *object);

// 0x633a00 (thiscall): vtable 0x703c80, +4 = +8 = 5, +0xc = 0, +0x10 = 1, +0x14 = +0x18 = +0x1c = -1.
u8 *__fastcall Class703c80_Ctor(u8 *object);

// 0x631280 (cdecl; object): the object's function pointer table (0x6312d0, 0x631340, 0x631300,
// 0x631380, 0x631440 at +0 to +0x10) and fields (+0x14, +0x68, +0x6c, +0x78-+0x80 = 0, +0x70 =
// 0x7039d0, +0x74 = 0x7b). Returns the object.
u8 *__cdecl Codec631280_Init(u8 *object);

// 0x5f9f80 (thiscall): the vtable pointer 0x7035d8 at +4, the byte at +0 and the words at +8, +0xa, +0xc cleared. Returns the object. 0x5f9f70 (cdecl): that for the global
// object at 0xfa78e8 (its static initialiser).
u8 *__fastcall Class7035d8_Ctor(u8 *object);

u8 *__cdecl Class7035d8_InitGlobal();

// 0x4254e0 (cdecl; object, from, to): Obj_RemoveFromRect of the object at +0x24 (from, to); then bit 14 of the dword at +0 is cleared.
// Returns 1.
u32 __cdecl Obj_RemoveRectAndClear14(u8 *object, u32 from, u32 to);
}
