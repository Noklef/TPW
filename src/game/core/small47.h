// The destructors of classes derived from the two list owning classes of small35 / small32 that only set their own vtable pointer and run the base destructor (twelve-byte
// stubs that jump to it), and their scalar deleting destructors: reimplementation of 0x41e2c0, 0x41e320, 0x41e4d0, 0x41e4e0, 0x41e5b0, 0x41e5c0, 0x5e3f60 and 0x5e3f90, and of
// 0x41d880, 0x41da50, 0x41ddc0, 0x41dea0, 0x41e170, 0x41e250, 0x5e3f40 and 0x5e3f70. Named by their vtables until the classes are known.
#pragma once

#include "../../../re/types.h"

extern "C" {
void __fastcall Class702ec0_Dtor(u8 *object);
void __fastcall Class702dd0_Dtor(u8 *object);
u8 *__fastcall Class702ec0_ScalarDtor(u8 *object, void *, u32 flags);
u8 *__fastcall Class702dd0_ScalarDtor(u8 *object, void *, u32 flags);

}  // extern "C"
