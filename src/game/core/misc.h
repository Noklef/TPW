// A few small functions that belong to no known unit yet: reimplementation of 0x633120, 0x633dc0, 0x6486e0, 0x6338b0, 0x633a00 and
// 0x631280 (all in the library range 0x630ca0-0x658000, to be placed when the libraries are mapped).
#pragma once

#include "../../../re/types.h"

extern "C" {


// 0x633120 (thiscall): Class703770_Ctor, then vtable 0x703c18 and +8 = +0xc = 0. Returns the object.
u8 *__fastcall Class703c18_Ctor(u8 *object);

// 0x633dc0 (thiscall): Class703770_Ctor, then +8, +0x14, +0x18, +0x1c, +0x20 = 0, +0x10 = 0x703744 and the
// vtables 0x703ca0 (+0) and 0x703c98 (+0xc). Returns the object.
u8 *__fastcall Class703ca0_Ctor(u8 *object);

// 0x6486e0 (cdecl; records, count): over the 0x20-byte records, the first with the greatest signed dword at
// +0x1c among those where that and the dword at +0x18 are above 0; 0 for none.
u8 *__cdecl Records32_MaxWeight(u8 *records, s32 count);


u8 *__fastcall Class703770_Ctor(u8 *object);

// 0x633a00 (thiscall): vtable 0x703c80, +4 = +8 = 5, +0xc = 0, +0x10 = 1, +0x14 = +0x18 = +0x1c = -1.
u8 *__fastcall Class703c80_Ctor(u8 *object);

// 0x631280 (cdecl; object): the object's function pointer table (0x6312d0, 0x631340, 0x631300,
// 0x631380, 0x631440 at +0 to +0x10) and fields (+0x14, +0x68, +0x6c, +0x78-+0x80 = 0, +0x70 =
// 0x7039d0, +0x74 = 0x7b). Returns the object.
u8 *__cdecl Codec631280_Init(u8 *object);

}
