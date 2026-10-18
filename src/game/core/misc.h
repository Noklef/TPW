// A few small functions that belong to no known unit yet: reimplementation of 0x402ef0, 0x4032b0, 0x40f300, 0x458910, 0x458940, 0x409260,
// 0x45be80, 0x633dc0, 0x6486e0, 0x458310, 0x4583d0, 0x633120, 0x424ed0 and 0x47f940.
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

// 0x458310 (thiscall; mostDerived, pops 4): a class with a virtual base. When most derived: vtable
// 0x6fe570 and the part at +0x60 gets vtable 0x6fdce8. Always: the virtual base (at the vtable's +4)
// gets vtable 0x6fdcd8, the dword before it = its offset - 0x60, and +0x50 = 0. Returns the object.
u8 *__fastcall Obj_CtorVirtual(u8 *object, void *, u32 mostDerived);

// 0x4583d0 (thiscall; a, b, c, pops 12): the scene flags touched (SceneFlags_Touch on 0x8bd508), then
// +0x2c = a, +0x30 = b, +0x34 = c. Returns a.
u32 __fastcall Obj_SetVectorAt2c(u8 *object, void *, u32 a, u32 b, u32 c);



// 0x409260 (thiscall): Obj_AdvanceUnless30 on the object at 0x785970; 0x7477e4 = bit 3 of the flags
// 0x7a1a14; and when the dword at +0x18 is set bit 1 of the flags is set. Returns the flags then, else 0.
u32 __fastcall Flags7a1a14_Update(const u8 *object);

// 0x45be80 (cdecl; object, from, to): the part at the object's +0x1c (a virtual base: the pointer plus the
// offset at +4 of its vtable; 0 for none) is removed from the renderer's grid ([0x87a6d0]) over the
// rectangle from.hi, from.lo to to.hi, to.lo (the positions' high and low words as x and y
// of the corners: Renderer_RemoveInRect); then bit 0 of the object's first dword is cleared. Returns that dword.
u32 __cdecl Obj_RemoveFromRect(u8 *object, u32 from, u32 to);

// 0x633120 (thiscall): Class703770_Ctor, then vtable 0x703c18 and +8 = +0xc = 0. Returns the object.
u8 *__fastcall Class703c18_Ctor(u8 *object);

// 0x633dc0 (thiscall): Class703770_Ctor, then +8, +0x14, +0x18, +0x1c, +0x20 = 0, +0x10 = 0x703744 and the
// vtables 0x703ca0 (+0) and 0x703c98 (+0xc). Returns the object.
u8 *__fastcall Class703ca0_Ctor(u8 *object);

// 0x6486e0 (cdecl; records, count): over the 0x20-byte records, the first with the greatest signed dword at
// +0x1c among those where that and the dword at +0x18 are above 0; 0 for none.
u8 *__cdecl Records32_MaxWeight(u8 *records, s32 count);

// 0x402ef0 (thiscall): when Obj_Get30 is 0, +0x3c += +0x40. Returns +0x30 if set, else +0x40.
u32 __fastcall Obj_AdvanceUnless30(u8 *object);

// 0x4032b0 (thiscall): Init_402f30, then +0x20 = 0 and +0x24 = 0x3ff00000 (the double 1.0 at +0x20).
u8 *__fastcall Obj_CtorOne(u8 *object);

// 0x47f940 (thiscall; value, pops 4): Fn_657d30 on the object, then vtable +0 = 0x6ff058 and +0x14 = value.
u8 *__fastcall Obj_CtorValue(u8 *object, void *, u32 value);

// 0x424ed0 (cdecl; pointer): bit 0 of the dword cleared; returns 1.
u32 __cdecl Dword_ClearBit0(u32 *p);

u8 *__fastcall Class703770_Ctor(u8 *object);

// 0x633a00 (thiscall): vtable 0x703c80, +4 = +8 = 5, +0xc = 0, +0x10 = 1, +0x14 = +0x18 = +0x1c = -1.
u8 *__fastcall Class703c80_Ctor(u8 *object);

// 0x631280 (cdecl; object): the object's function pointer table (0x6312d0, 0x631340, 0x631300,
// 0x631380, 0x631440 at +0 to +0x10) and fields (+0x14, +0x68, +0x6c, +0x78-+0x80 = 0, +0x70 =
// 0x7039d0, +0x74 = 0x7b). Returns the object.
u8 *__cdecl Codec631280_Init(u8 *object);

// 0x4254e0 (cdecl; object, from, to): Obj_RemoveFromRect of the object at +0x24 (from, to); then bit 14 of the dword at +0 is cleared.
// Returns 1.
u32 __cdecl Obj_RemoveRectAndClear14(u8 *object, u32 from, u32 to);
}
