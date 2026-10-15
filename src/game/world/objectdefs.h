// The object definitions (loaded by ObjectDefs_Load 0x413c10 from .sam files): reimplementation
// of the lookups 0x412e90, 0x412ef0 and 0x412f50. The set holds up to 0x14a definition pointers
// from +0, their count at +0x528 and a one-entry cache, +0x730 (u16) the id last found and
// +0x734 its definition; a definition starts with its u16 id.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x412e90: the definition with id (the cached one when id is the cached id, whatever it
// holds; else the first in the list, which is then cached), or 0.
u8 *__fastcall ObjectDefs_Find(u8 *defs, void *, u16 id);

// 0x412ef0 / 0x412f50: the found definition's +0x390 / +0x410 part, or 0.
u8 *__fastcall ObjectDefs_FindAt390(u8 *defs, void *, u16 id);
u8 *__fastcall ObjectDefs_FindAt410(u8 *defs, void *, u16 id);
}
