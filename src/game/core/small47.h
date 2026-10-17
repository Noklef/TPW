// The destructors of the classes derived from the 0x702cb0 list owning class (small32's) that only set their own vtable pointer and run the base destructor (twelve-byte stubs that
// jump to it, 0x5e3f60 and 0x5e3f90), and their scalar deleting destructors (0x5e3f40 and 0x5e3f70). Named by their vtables until the classes are known.
#pragma once

#include "../../../re/types.h"

extern "C" {
// Each sets the vtable pointer named and runs Class702cb0_Dtor on the object.
void __fastcall Class702ec0_Dtor(u8 *object);
void __fastcall Class702dd0_Dtor(u8 *object);
// The same, then the object is freed when bit 0 of flags is set. Returns the object.
u8 *__fastcall Class702ec0_ScalarDtor(u8 *object, void *, u32 flags);
u8 *__fastcall Class702dd0_ScalarDtor(u8 *object, void *, u32 flags);

}  // extern "C"
