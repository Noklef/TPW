// The find of four std::map instantiations (lower bound, then the end if the key is below the found node's): reimplementation of 0x410610, 0x47c6a0, 0x5e76c0 (thiscall on
// the map; out, key pointer, pops 8: *out = the node found or the head node (the end); returns out) and 0x5af780 (thiscall on an object whose map is at +0x28; the word key
// by value, pops 4: whether it is there). The lower bounds are Tree16a_LowerBound, Tree32_LowerBound and Tree16b_LowerBound (small23).
#pragma once

#include "../../../re/types.h"

extern "C" {

u8 **__fastcall Tree16a_Find(u8 *tree, void *, u8 **out, const u16 *key);
u8 **__fastcall Tree32_Find(u8 *tree, void *, u8 **out, const u32 *key);
u8 **__fastcall Tree16b_Find(u8 *tree, void *, u8 **out, const u16 *key);
u32 __fastcall Obj_HasKey5af780(u8 *object, void *, u32 key);

}  // extern "C"
