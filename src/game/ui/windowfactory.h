// The GUI library's window factory (0x65dd7b: a child window of one of 13 kinds, each a block of its size from the game's heap built by its
// constructor and put last among the parent's children). Reimplementation of 0x65dd7b. The name is unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x65dd7b (thiscall on the parent (kind, flags, id, left, top, right, bottom: words), pops 28; name unconfirmed): 0 for a kind outside
// 1-13. Otherwise a block (Mem_Alloc) of the kind's size built by its constructor (parent, flags, id and the words), 0 when none was got:
// 1 Window_Ctor (0x138; with flags & 0x100 the new window's +0x64 the parent), 2 Window705a50_Ctor (0x15c), 3 Window705b20_Ctor (0x15c),
// 4 Window705950_Ctor (0x158), 5 Window705a20_Ctor (0x168), 6 Window705c58_Ctor (0x140), 7 Window7059f0_Ctor (0x178), 8 RadioGroup_Ctor
// (0x13c), 9 Window705c28_Ctor (0x154), 10 Window705bf8_Ctor (0x150), 11 Window705bc8_Ctor (0x168), 12 Window705af0_Ctor (0x158), 13
// Window705b98_Ctor (0x13c). The new window goes last among the parent's children (the first +0x5c, the next +0x54; its previous +0x58,
// 0 for the first), not checked for 0 (sic: a failed allocation faults there, and at +0x64 for kind 1 with flags & 0x100). Returns it.
u8 *__fastcall Window_CreateChild(u8 *parent, void *, u32 kind, u32 flags, u32 id, u32 left, u32 top, u32 right, u32 bottom);
}
