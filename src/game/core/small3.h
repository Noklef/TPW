// More tiny functions on globals and objects: reimplementation of 0x52f860, 0x52fab0, 0x52faf0, 0x52fba0, 0x557d60, 0x573860,
// 0x4f0140, 0x5cde30 and 0x5ce910. Named by what they do until the classes are known.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x52f860 (cdecl; value): whether the dword at 0x81ae2c equals the value.
u32 __cdecl Global81ae2c_Equals(u32 value);

// 0x52fab0 / 0x52faf0 (cdecl): the dword at 0x81b0cc / 0x81ecac when the dword at 0x8187f8 is 0, else 0.
u32 __cdecl Global81b0cc_IfNoFlag();
u32 __cdecl Global81ecac_IfNoFlag();

// 0x52fba0 (cdecl; amount): the dword at 0x81af4c += the amount. Returns the amount.
u32 __cdecl Global81af4c_Add(u32 amount);

// 0x557d60 (cdecl; source, a, b, c): *a, *b and *c = the source's dwords at +0x30, +0x34 and +0x38. Returns the last.
u32 __cdecl Obj_Read30_34_38(const u8 *source, u32 *a, u32 *b, u32 *c);

// 0x573860 (cdecl): the dword at 0x4c4 of what the dword at 0x879a24 points to.
u32 __cdecl Global879a24_Get4c4();

// 0x4f0140 / 0x5cde30 (thiscall): through the pointer 8 / 4 before the object, a vtable pointer, the dword at +4 of that is an
// offset to a virtual base: the dword 8 / 4 before the object plus that offset is set to 0x7099dc / 0x70995c. Returns that
// vtable pointer.
u32 __fastcall VirtualBase_SetVtbl7099dc(u8 *object);
u32 __fastcall VirtualBase_SetVtbl70995c(u8 *object);

// 0x5ce910 (thiscall; out, unused, pops 8): a list cursor (a node pointer at +0) moves to the node's next (its first dword); the
// node it left is stored at *out. Returns out.
u32 *__fastcall ListCursor_Next(u32 *cursor, void *, u32 *out, u32 unused);

}  // extern "C"
