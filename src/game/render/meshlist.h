// Readying a zero-ended array of meshes: reimplementation of 0x5744d0 and 0x574110 (x87 at the game's 24-bit precision).
//
// The meshes (at least 0x5c bytes) have their flags at +0x50 and a link at +0x58. The object at
// [0x87a248] keeps +0x30 = the address of the last mesh with flag 0x20's +0x38; the meshes without
// flag 0x10000000 are pushed on the list headed at 0x879dec. The object's +0x34-+0x3c is a direction (0x574110, set by the drawing
// at 0x57ab29 from the camera's position to a point). The list's entries look like lights (0x5741b0 walks it by kind +0x50 & 0xf: 2,
// 4 and 6, adding their colours at +4-+0x10 into 0x879db0), so the names may be wrong.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x5744d0 (thiscall on the object; list): for each mesh in the zero-ended array: flag 0x20 sets
// the object's +0x30 = mesh + 0x38; without flag 0x10000000, the mesh's +0x58 = the list head
// (0x879dec) and the head = the mesh (the flag isn't set, so a mesh readied twice links to itself
// (sic)). An empty array does nothing. Returns 1.
u32 __fastcall Meshes_Ready(u8 *object, void *, const u32 *list);

// 0x574110 (thiscall on the object; x, y, z: pops 0xc; name unconfirmed): +0x34 / +0x38 / +0x3c = (x, y, z) made unit length:
// len = sqrt((x * x + y * y) + z * z), each (1 / len) * coordinate; when len is 0 or a NaN, the three copied as they are (as dwords).
void __fastcall Meshes_SetDirection(u8 *object, void *, u32 x, u32 y, u32 z);
}
