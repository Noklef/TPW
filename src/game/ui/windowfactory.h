// The GUI library's window factory (0x65dd7b: a child window of one of 13 kinds, each a block of its size from the game's heap built by its
// constructor and put last among the parent's children). Reimplementation of 0x65dd7b. The name is unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x65dd7b (thiscall on the parent (kind, flags, id, left, top, right, bottom: words), pops 28; name unconfirmed): 0 for a kind outside
// 1-13. Otherwise a block (Mem_Alloc) of the kind's size built by its constructor (parent, flags, id and the words), 0 when none was got;
// by kind the size and the constructor's vtable: 1 0x138, 0x705980 (Window_Ctor; with flags & 0x100 the new window's +0x64 the parent),
// 2 0x15c, 0x705a50; 3 0x15c, 0x705b20; 4 0x158, 0x705950; 5 0x168, 0x705a20; 6 0x140, 0x705c58; 7 0x178, 0x7059f0; 8 0x13c, 0x705ac0
// (the radio group); 9 0x154, 0x705c28; 10 0x150, 0x705bf8; 11 0x168, 0x705bc8; 12 0x158, 0x705af0; 13 0x13c, 0x705b98 (each unit
// ui/window<vtable>). The new window goes last among the parent's children (the first +0x5c, the next +0x54; its previous +0x58,
// 0 for the first), not checked for 0 (sic: a failed allocation faults there, and at +0x64 for kind 1 with flags & 0x100). Returns it.
u8 *__fastcall Window_CreateChild(u8 *parent, void *, u32 kind, u32 flags, u32 id, u32 left, u32 top, u32 right, u32 bottom);
}
