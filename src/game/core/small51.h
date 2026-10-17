// A class destructor that leaves the global list of its kind, and the test that a person queue has room: reimplementation of 0x5f4400 and 0x4dda20. Named by what they do until
// the classes are known.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x5f4400 (thiscall; flags, pops 4): sets the vtable pointer 0x703088, takes the object out of the list at [0xfa6e38] (head +4, tail +8, count +0xc; nodes {value, next, previous}:
// the first node holding the object is unlinked and freed), removes the pointer at +0x28 from the array at 0x879ce0 (PtrArray_RemoveAndFree, which frees it) and frees the object
// when bit 0 of the flags is set. Returns the object.
u8 *__fastcall Class703088_ScalarDtor(u8 *object, void *, u32 flags);

// 0x4dda20 (thiscall): whether the queue of persons (Object_QueuePosition for the person 0, i.e. its length) is shorter than the dword at +0x40 times 4 (unsigned).
u32 __fastcall Object_QueueHasRoom(u8 *object);

}  // extern "C"
