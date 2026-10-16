// Tiny deleting destructors, constructors and clears: reimplementation of 0x42a090, 0x56a660, 0x55f1e0, 0x5c8d80, 0x5cde50, 0x5d0690,
// 0x5eb170 and 0x4d05d0. Named by what they do until the classes are known.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x42a090 (thiscall; flags, pops 4): the vtable pointer at +0 = 0x6fdce8; when bit 0 of the flags is set the block 0x60 bytes before the
// object is freed. Returns that address (the object - 0x60), freed or not.
u8 *__fastcall Deleter_Minus60_42a090(u8 *object, void *, u32 flags);

// 0x56a660 (thiscall; flags, pops 4): Vtbl701260_Set_565230; when bit 0 of the flags is set the block 8 bytes before the object is freed.
// Returns that address (the object - 8).
u8 *__fastcall Deleter_Minus8_56a660(u8 *object, void *, u32 flags);

// 0x55f1e0 (thiscall): the block at +0xc, when there is one, is freed; the dwords at +0, +4, +8 and +0xc = 0.
void __fastcall BufferC_FreeAndClear(u8 *object);

// 0x5c8d80 (thiscall): Class70295c_Ctor, then the dwords at +0x18, +0x1c and +0x420 = 0, the vtable pointer at +0 = 0x702958 and the 256
// dwords from +0x20 = 0. Returns the object.
u8 *__fastcall Class702958_Ctor(u8 *object);

// 0x5cde50 (thiscall; source, pops 4): like ListHead_Ctor but the byte at +0 is copied from the byte the source points to and the first
// dword at +0xc is not set. Returns the object.
u8 *__fastcall ListHead_CtorFrom(u8 *object, void *, const u8 *source);

// 0x5d0690 (stdcall; a, b): a new block of 12 bytes (otherwise left as it is) with a at +0 and b at +4, each the block itself when 0. Returns it.
u8 *__stdcall Node0c_New(u32 a, u32 b);

// 0x5eb170 (thiscall): Array3D_Free of the members at +0x2c, +0x3c, +0x4c and +0x5c.
void __fastcall Array3Ds_Free4(u8 *object);

// 0x4d05d0 (thiscall; value, pops 4): the dword at +0x118 = value (the debug print is not reproduced).
void __fastcall Obj_Set118(u8 *object, void *, u32 value);

}  // extern "C"
