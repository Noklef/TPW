// A thing's ingredient record and a stream class constructor: reimplementation of 0x4e2200 and 0x5667d0. Named by what they do until the classes are known.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x4e2200 (thiscall on the thing; the object definition by the id word at +0xe, ObjectDefs_Find on 0x7890a0): the definition's dword at +0x158 (1 to 4) picks the record 0 to
// 3 of the table at [0x78ba8c] (Records_DataIfTerminated); any other value gives 0.
u8 *__fastcall Thing_IngredientRecord(u8 *thing);

// 0x5667d0 (thiscall; most derived flag, pops 4): with the flag set the vtable 0x701378 is stored and the base class constructor (Class701260_Ctor) runs on the part at +0x2c;
// then the virtual base (offset from the word at +4 of the vtable) gets the vtable 0x701330, the word before it the distance to that base less 0x2c, and the fields +8, +0x18, +4 are
// cleared, +0x10 is 2, +0x14 is 1, +0x20 is -1 and the word at the virtual base + 0xc is 0x3000. Returns the object.
u8 *__fastcall Class701378_Ctor(u8 *object, void *, u32 mostDerived);

}  // extern "C"
