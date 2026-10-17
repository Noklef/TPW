// A batch opening and a release of saved pairs: reimplementation of 0x565310 and 0x57b070. Named by what they do until the classes are known.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x565310 (thiscall; mode, a, kind, b, c, pops 20): the draw buffer at [0x8bcbc4] resumes a batch for the mode (DrawBuffer_Resume) and the object keeps it at +0x20; none gives 0.
// The batch gets b at +4, a at +0x18, the kind at +0x1c, c at +8 and -1 at +0x34; the object's size at +0x1c is 0x18 for kind 0xc4, 0x20 for 0x1c4, 0x28 for 0x2c4, 0x30 for
// 0x3c4 and 0 for anything else. Returns the batch.
u8 *__fastcall DrawBatch_Open(u8 *object, void *, u32 mode, u32 a, u32 kind, u32 b, u32 c);

// 0x57b070 (thiscall on the object): with flag 0x20000000 of the flags (+4) and a positive count (+0x12c) the saved pairs {value, pointer} at +0x124 are written back (the value to the
// word the pointer names, the last pair first) and the array freed (Mem_Free); the 0x10-byte elements at +0x128 (their count the dword before them) are destroyed (the dtor at
// 0x575bf0) and their block freed from 4 bytes before; the flag is cleared. In every case the count is cleared. Returns 1.
u32 __fastcall Object_ReleaseSavedPairs(u8 *object);

}  // extern "C"
