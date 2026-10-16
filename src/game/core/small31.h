// Three destructors of the class with the vtable pointer 0x703780 that release their member: reimplementation of 0x617a80, 0x617b80 and 0x6180c0 (identical but for the
// exception tables). Named by their vtable pointer until the class is known.
#pragma once

#include "../../../re/types.h"

extern "C" {

// (thiscall): the vtable pointer 0x703780 is set, the object at +8 (when there is one) gets its slot 0 called (no argument), and the vtable pointer 0x703770 is set.
void __fastcall Class703780_DtorA(u8 *object);
void __fastcall Class703780_DtorB(u8 *object);
void __fastcall Class703780_DtorC(u8 *object);

}  // extern "C"
