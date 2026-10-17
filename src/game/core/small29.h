// Destructors of three classes that take themselves off their parent's list and destroy three strings, and the two resets that release a shared array: reimplementation of
// 0x5bdc10, 0x5be320, 0x5be750, 0x5b97a0 and 0x5b9820. Named by their vtable pointers until the classes are known.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x5bdc10 / 0x5be320 / 0x5be750 (thiscall): the vtable pointer 0x702938 / 0x702940 / 0x702948 is set; the object is taken off the list the object at +4 has at +0x2c (nodes
// {object, next, previous}; head +4, tail +8, count +0xc; the node is freed); then the strings at +0x44, +0x20 and +0xc / +0x34, +0x20 and +0xc / +0x4c, +0x20 and +0xc are destroyed (WString_Dtor / String_Dtor / WString_Dtor).
void __fastcall Class702938_Dtor(u8 *object);
void __fastcall Class702940_Dtor(u8 *object);
void __fastcall Class702948_Dtor(u8 *object);

// 0x5be0e0 / 0x5be530: the same for the classes with the vtable pointers 0x70293c (the parent list only when the parent at +4 is set; wide strings at +0x94, +0x34, +0x20, +0xc and a
// narrow one at +0x7c) and 0x702944 (wide strings at +0x6c, +0x54, +0x40, +0x24, +0x10).
void __fastcall Class70293c_Dtor(u8 *object);
void __fastcall Class702944_Dtor(u8 *object);

// 0x5b97a0 / 0x5b9820 (thiscall; a, b, pops 8): +4 = -1, +8, +0xc, +0x10, +0x14 and +0x18 cleared, +0x1c = a, +0x20 = b; the shared record at +0x30, when there is one, loses a use
// and at 0 its array (elements of 0x26 / 0x34 bytes destroyed by the function the dword at 0x6fd508 / 0x6fd510 holds) and the record are freed; +0x30 is cleared.
void __fastcall Obj_ResetAndRelease26(u8 *object, void *, u32 a, u32 b);
void __fastcall Obj_ResetAndRelease34(u8 *object, void *, u32 a, u32 b);

}  // extern "C"
