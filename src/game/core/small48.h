// Two users of the map insert over the 16-bit keys (sentinel 0xf7c970, StdMapG_Insert) and a class destructor that frees a list of blocks: reimplementation of 0x5b1650,
// 0x5af740 and 0x617f40. Named by what they do until the classes are known.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x5b1650 (thiscall on the tree; out, value, pops 8): StdMapG_Insert's result {iterator, inserted byte} copied to out. Returns out.
u32 *__fastcall StdMapG_InsertCopy(u8 *tree, void *, u32 *out, const u16 *value);

// 0x5af740 (thiscall on an object whose tree is at +0x28; key (a 16-bit value passed on the stack), amount, pops 8): inserts the key; true (and +0x1c grows by the amount)
// when it was new, else false.
u8 __fastcall Object_AddKeyed(u8 *object, void *, u32 key, s32 amount);

// 0x617f40 (thiscall destructor): sets the vtable pointer 0x7037b8, frees the block at +8 of every node of the list at +8 (head, size at +0xc), then every node and the
// head, and clears the pointer and the size.
void __fastcall Class7037b8_Dtor(u8 *object);

}  // extern "C"
