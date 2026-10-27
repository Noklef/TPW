// A wide-text writer (vtable 0x70a0c0, set at 0x6acdb5 / 0x6ad0cd; the class name is invented): +4 the cursor (u16 cells), +8 the end,
// +0xc a table holder (its slot 0 releases it), +0x10 a byte set once the cursor reached the end. Its slot 4 is Obj_PutWord (methods.h).
// Reimplementation of 0x6b00e0; the name is unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x6b00e0 (slot 1, thiscall): with a cursor (+4, read once): the full byte (+0x10) set when it is 0 and the cursor is at the end
// (+8); then a 0 cell at the cursor while the byte is 0, else over the cell before it; the cursor cleared. Then a holder (+0xc) is
// cleared and released through its slot 0.
void __fastcall WideWriter_Finish(u8 *writer);

}
