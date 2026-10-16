// Constructors and destructors of classes that hold a list of objects behind a pointer: reimplementation of 0x5da200 / 0x5e3c20 (constructors) and 0x5da2f0 / 0x5e03b0 /
// 0x5e3d10 (destructors). Each constructor is Class7029d0_Ctor on the object, its vtable pointer, and a new 0x10-byte list (its own vtable pointer, the head, tail and count
// cleared; none when the allocation fails) at +4 / +0x18. Each destructor sets the vtable pointer, empties the list (each object popped from the head is deleted through its slot 0
// with 1), deletes the list itself (slot 0 with 1), clears the pointer and releases the object (Class7029d0_Release; Class702d48_Release for the 0x702cb0 class).
#pragma once

#include "../../../re/types.h"

extern "C" {

u8 *__fastcall Class702b08_Ctor(u8 *object);
u8 *__fastcall Class702d78_Ctor(u8 *object);
void __fastcall Class702b08_Dtor(u8 *object);
void __fastcall Class702cb0_Dtor(u8 *object);
void __fastcall Class702d78_Dtor(u8 *object);

}  // extern "C"
