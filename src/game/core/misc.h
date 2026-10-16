// A few small functions that belong to no known unit yet: reimplementation of 0x402ef0, 0x4032b0, 0x40f300, 0x458910, 0x458940, 0x4a9350, 0x4b4670, 0x458310, 0x4583d0, 0x4bcec0, 0x4f36c0, 0x52fc30, 0x5997b0, 0x54b290, 0x56b340, 0x56e780, 0x583360, 0x5646c0, 0x564ba0, 0x56b570, 0x5651c0, 0x4d09a0, 0x52fbf0, 0x424ed0, 0x47f940, 0x4922d0, 0x4d4170, 0x4d9600, 0x4e01f0,
// 0x4e1c60, 0x4f00f0, 0x4f6020, 0x4f6820, 0x4f6850, 0x56a510, 0x56b630 and 0x56b830.
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

// 0x5651c0 (thiscall): vtable 0x701260, +4 = +8 = +0x18 = +0x1c = +0x20 = +0x24 = 0, +0x14 = -1; and
// 0x879ab4 = 1 when the display adapter record at [0x879a24] has vendor +0x428 = 0x10de and the driver
// kind 0x8bd524 is 1. Returns the object.
u8 *__fastcall Class701260_Ctor(u8 *object);

// 0x5646c0 (thiscall; mostDerived, pops 4): a class with a virtual base like 0x458310. When most derived:
// vtable 0x701188 and the part at +0x18 gets vtable 0x701178. Always: the virtual base (the vtable's
// +4) gets vtable 0x701168, the dword before it = its offset - 0x18, and +4, +8, +0xc and +0x10 = 0.
// Returns the object.
u8 *__fastcall Obj_CtorVirtual5646c0(u8 *object, void *, u32 mostDerived);

// 0x564ba0 (thiscall; mostDerived, pops 4): the same for vtable 0x7011d8 (part at +0x2c: 0x701178), the
// base's vtable 0x7011c8, the dword before it = its offset - 0x2c, and +4 to +0x24 zeroed (+4-+0x1c,
// then +0x24 and +0x20). Returns the object.
u8 *__fastcall Obj_CtorVirtual564ba0(u8 *object, void *, u32 mostDerived);

// 0x56b570 (thiscall; key, a, b, c, pops 16): the array of pointers at +8 (count +0x1c) searched
// backwards for the key; when found the key's dwords at +0x14, +0x18, +0x1c = a, b, c and 1 is
// returned, else 0.
u32 __fastcall PtrArray_SetVectorOf(const u8 *list, void *, u8 *key, u32 a, u32 b, u32 c);

// 0x56e780 (thiscall; low, first, high, last, pops 16): the extents of a column range. +0x20 =
// min(+0x20, first) and +0x24 = max(+0x24, last) (signed); then for each column from first to last
// inclusive, the signed byte at +0x28 + column is lowered to the low byte of `low` when above it
// (compared as a dword) and the byte at +0xa8 + column raised to the low byte of `high` when below.
void __fastcall Extents_Include(u8 *extents, void *, s32 low, s32 first, s32 high, s32 last);

// 0x54b290 (cdecl; owner): every chunk on the list at owner +0xbc is freed: its nodes (the list at
// chunk +0x1c, next at node +0x14) have bit 0 of their first dword cleared and 0x877b74 stepped down,
// then the owner's head = chunk +0x24, bit 0 of the chunk's first dword cleared and 0x877b84 stepped down.
void __cdecl Pool_FreeAll(u8 *owner);

// 0x56b340 (thiscall): 1 when bit 0 of the first dword is already clear. Else it is cleared and the part
// at +4 (a virtual base: the pointer plus the offset at +4 of its vtable; 0 for none) removed from the
// renderer's tag 0x60 list (Renderer_RemoveFromSlot): 1 if that succeeded, 0 if it failed or there is
// no renderer ([0x87a6d0]).
u32 __fastcall Obj_ReleaseFromRenderer(u8 *object);

// 0x583360 (thiscall): the highest set bit of the masks at +0x8fc, +0x904 and +0x90c (0 for a zero mask)
// stored at +0x900, +0x908 and +0x910, and their union at +0x918, which is returned.
u32 __fastcall Masks_TopBits(u8 *object);

// 0x4f36c0 (thiscall): the number of 36-byte elements between the pointers at +4 and +8 (a signed
// division); 0 when +4 is null.
s32 __fastcall Vector36_Size(const u8 *vector);

// 0x52fc30 (cdecl): the pair table at 0x81b740 (count 0x820a8c) copied to 0x818d98; then 0x820a90 = the
// count and 0x820a94 = 0. Returns the count when above 0, else 0.
s32 __cdecl PairTable_Commit();

// 0x5997b0 (cdecl; id): the table of 0x20-byte entries at 0x768fb8, ended by an id of 0x270f, searched
// for the id; the dword at +0x1c of the entry found, or 0x17f for none.
u32 __cdecl Table768fb8_Lookup(u32 id);

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

// 0x56a510 (stdcall; x, y, out, pops 12): *out = the dword at (y * width + x) in the software
// renderer's buffer (0x879dac: +8 the buffer; width 0x8bcbd0); returns 1.
u32 __stdcall SoftRenderer_PixelAt(s32 x, s32 y, u32 *out);

// 0x56b630 (cdecl): the sixteen dwords at 0x879d08 = 16, 15 ... 1; returns 0.
u32 __cdecl Table879d08_Fill();

// 0x56b830 (thiscall; a, b, pops 8): +0x30 = a and +0x34 = b; returns a.
u32 __fastcall Obj_Set30_34(u8 *object, void *, u32 a, u32 b);
}
