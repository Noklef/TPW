// A display format record filler, a list node splice and a setter of the global game object: reimplementation of 0x4925d0, 0x5d0510 and 0x4b0aa0.
// Named by what they do until the callers are known.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x4925d0 (cdecl; a, out, c, d): when out is not 0 it is filled from the format record the dword at 0x8bd554 indexes (32 bytes each from 0x8bd558): +0x18 = a, +0x1c = c,
// +0x20 = d, +0x10 = the record's dword 3, +0 / +4 / +8 / +0xc = its dwords 4 / 5 / 6 / 7, +0x24 = (dword 3 >> 3) * c, +0x14 = its dword 1 & 0x20.
void __cdecl Format_FillInfo(u32 a, u32 *out, u32 c, u32 d);

// 0x5d0510 (thiscall on a list: +8 count; node a, list x, node c, pops 12): when c is neither a nor the node before a, the node c moves from its place (in list x) to in front
// of a (circular lists: +0 next, +4 previous); the count of this list rises, that of x falls.
void __fastcall List_SpliceOne(u8 *list, void *, u8 *a, u8 *x, u8 *c);

// 0x4b0aa0 (thiscall): the global game object (the dword at 0x7c2658) is given the value at +0x28 (Set198_4e21e0), then the signed byte at +0x24 times 50 (in the low word of
// the argument; the upper half is the value just returned, as the original leaves it) as its quality of goods, then the value at +0x2c (Obj_Set194).
void __fastcall Obj_ApplyToGlobalObject(u8 *object);

}  // extern "C"
