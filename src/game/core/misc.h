// A few small functions that belong to no known unit yet: reimplementation of 0x402ef0, 0x4032b0, 0x40f300, 0x458910, 0x458940, 0x4a9350, 0x4b4670, 0x4bcec0, 0x424ed0, 0x47f940, 0x4922d0, 0x4d4170, 0x4d9600, 0x4e01f0,
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
