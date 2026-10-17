// A holder of four arrays and a reader of eleven dwords: reimplementation of 0x5eb090 and 0x5eb1a0. Named by what they do until the classes are known.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x5eb090 (thiscall destructor-like): Array3D_Free on the arrays at +0x2c, +0x3c, +0x4c and +0x5c, then for +0x5c, +0x4c, +0x3c and +0x2c in that order the block at +0xc of
// each freed when still set and its four dwords cleared.
void __fastcall ArrayHolder_Clear(u8 *object);

// 0x5eb1a0 (thiscall; stream, pops 4): the eleven dwords at +0 .. +0x28 read from the stream (Object_CallSlot8Product with 4, 1). Returns true.
u8 __fastcall Eleven_Read(u8 *object, void *, u8 *stream);

}  // extern "C"
