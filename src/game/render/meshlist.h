// Readying a zero-ended array of meshes: reimplementation of 0x5744d0.
//
// The meshes (at least 0x5c bytes) have their flags at +0x50 and a link at +0x58. The object at
// [0x87a248] keeps +0x30 = the address of the last mesh with flag 0x20's +0x38; the meshes without
// flag 0x10000000 are pushed on the list headed at 0x879dec.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x5744d0 (thiscall on the object; list): for each mesh in the zero-ended array: flag 0x20 sets
// the object's +0x30 = mesh + 0x38; without flag 0x10000000, the mesh's +0x58 = the list head
// (0x879dec) and the head = the mesh (the flag isn't set, so a mesh readied twice links to itself
// (sic)). An empty array does nothing. Returns 1.
u32 __fastcall Meshes_Ready(u8 *object, void *, const u32 *list);
}
