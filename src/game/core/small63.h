// A thing's constructor: reimplementation of 0x50afe0. Named by what it does until the class is known.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x50afe0 (thiscall on the thing; cell id, kind, pops 8; the print of the creation is left out): the handle word at +0 is cleared, the kind byte stored at +2, the byte at +3
// and the words at +8 and +0xa cleared; the thing takes a slot in the world's table (World_AddThing on [0x80239c]) and its handle; +4 and +6 are 0x80, the column byte at +5 is the cell
// id less 1 (to 7 bits) and the row byte at +7 that id less 1 shifted down 7; the thing goes at the head of the cell's chain (Obj_ReplaceHandle24 on the cell at [0x8023a0] +
// id * 0x44 - 0x44). Returns the thing.
u8 *__fastcall Thing_Construct(u8 *thing, void *, u32 cell, u32 kind);

}  // extern "C"
