// A class that owns a list of objects and one more object: reimplementation of 0x41c8b0. Named by its vtable until the class is known.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x41c8b0 (thiscall destructor on the object; not freed): sets the vtable pointer to 0x6fd9d8, deletes the object at +0x3c through its slot 0xc (with 1) and clears the
// pointer, and when there is a list at +0x2c empties it (each held object popped from the head is deleted through its slot 0x2c with 1; a node without a link ends the pop
// of that round with nothing deleted) and deletes the list itself through its slot 0 (with 1); then clears +0x2c (only when there was a list) and frees the strings at
// +0x18 and +4 (String_Dtor).
void __fastcall Class6fd9d8_Dtor(u8 *object);

}  // extern "C"
