// The live objects list: reimplementation of 0x5775d0 and 0x577600.
//
// Classes sharing the abstract virtual base with vtable 0x6fdce8 (the fonts, 0x48f1ff; the
// decoded textures, 0x56b168; some engine parts) add that base to a global list of 0x200 when
// built and take it out when destroyed. The list (0x87a6d8, count 0x87aed8) is only ever written:
// nothing in the exe walks it.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x5775d0 (cdecl; object): appended when there is room (the rest are dropped); 1 either way.
u32 __cdecl LiveObjects_Add(void *object);

// 0x577600 (cdecl; object): the last match, searching from the end, is replaced by the last entry
// and the count drops; 1, or 0 when it is not there.
u32 __cdecl LiveObjects_Remove(void *object);
}
