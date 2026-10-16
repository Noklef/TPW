// Methods over counted objects (a use count at +4, saturating at -1) and two constructors: reimplementation of 0x4f5d10, 0x4ef7f0, 0x5d0dc0, 0x4f21a0, 0x4f1520,
// 0x4f5d40, 0x5b3110, 0x618130 and 0x5b9020. The STL lock around the counts is left out. Named by what they do until the classes are known.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x4f5d10 (thiscall on a counted object): the count at +4 is raised unless it is -1.
void __fastcall Counted_AddRef(u8 *counted);

// 0x4ef7f0 / 0x5d0dc0 (thiscall; out, pops 4): the counted object the dword at +0x20 / +0x34 points to is stored in out and gets a use. Returns out.
u8 **__fastcall Obj_ShareMember20(u8 *object, void *, u8 **out);
u8 **__fastcall Obj_ShareMember34(u8 *object, void *, u8 **out);

// 0x4f21a0 (thiscall on a dword variable): the variable is given the next id (the dword at 0xfb1dd4 raised) when it is 0. Returns it.
u32 __fastcall Id_Get(u32 *id);

// 0x4f1520 (thiscall; a, pops 4): +0 = a, +8 and +0xc from the dwords at 0x7099f0 and 0x7099f4, +0x10 from the dword at 0xfb34b0. Returns the object.
u8 *__fastcall Obj_Init4f1520(u8 *object, void *, u32 a);

// 0x4f5d40 (thiscall; from, pops 4): the counted object from points to is shared (+0 = it, a use added). Returns the object.
u8 *__fastcall Counted_CopyCtor(u8 *object, void *, const u8 *from);

// 0x5b3110 (stdcall; a, b, pops 8): a new 0x14-byte node with a at +4 and b at +0x10 (the rest as the allocator left it).
u8 *__stdcall Node14_New(u32 a, u32 b);

// 0x618130 (thiscall; from, pops 4): +0 = the byte at from; a new 0xc-byte sentinel node pointing at itself twice is allocated, +4 = it, +8 = 0. Returns the object.
u8 *__fastcall Sentinel_Ctor(u8 *object, void *, const u8 *from);

// 0x5b9020 (thiscall; out, pops 4): out is constructed as a wide string from the text pointer at +0x54 (WString_CtorStr). Returns out.
u8 *__fastcall Obj_NameToWString(u8 *object, void *, u8 *out);

// 0x4f7970 / 0x5d15d0 (cdecl): the counted object the dword at 0x7ced44 / 0xf827dc points to loses a use (as Counted_Release) and is deleted (slot 0 with 1) when it has none left;
// the dword is cleared. (A null dword faults.)
void __cdecl Global7ced44_Release();
void __cdecl Globalf827dc_Release();

}  // extern "C"
