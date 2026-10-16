// More tiny functions (a getter behind the object, float constants set in globals, a colour split into floats): reimplementation
// of 0x57ac90, 0x57aca0, 0x57cb10, 0x57cb30, 0x57cb70 and 0x5832e0. Named by what they do until the classes are known.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x57ac90 (thiscall): the dword 0x14c before the object (the object is a member 0x14c into its owner).
u32 __fastcall Obj_GetBefore14c(const u8 *object);

// 0x57aca0 / 0x57cb30 (cdecl): the dword at 0x8bb628 / 0x8bba78 = 1.0f. Both return 1.0f's bits.
u32 __cdecl Global8bb628_SetOne();
u32 __cdecl Global8bba78_SetOne();

// 0x57cb10 (thiscall; a, b, c, pops 12): +0x14 = a, +0x18 = b, +0x10 = c. Returns b.
u32 __fastcall Obj_Set14_18_10(u8 *object, void *, u32 a, u32 b, u32 c);

// 0x57cb70 (cdecl): 0x8bcb8c = 1.0f and the dwords at 0x8bcb80, 0x8bcb84 and 0x8bcb88 = 1. Returns 1.
u32 __cdecl Globals8bcb80_SetOne();

// 0x5832e0 (thiscall; colour, pops 4): +0x8b4 = the colour and the floats at +0x8b8, +0x8bc, +0x8c0 and +0x8c4 = its
// bytes from the top (each left in place: 0xff000000, 0xff0000, 0xff00, 0xff). Returns the lowest byte.
u32 __fastcall Obj_SetColour8b4(u8 *object, void *, u32 colour);

}  // extern "C"
