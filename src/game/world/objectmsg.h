// The object's message handler (the part of Object_HandleMessage 0x4dd320): reimplementation. Named by what it does.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x4dd320 (thiscall on the object; message, pops 4): the message's number comes from its slot 0. For 0xb (the end of the day) each of the object's six history rings
// (temp dword at +0x1a8, +0x230, +0x2b8, +0x70, +0xf8, +0x340, in this order; then the data from +4, the current index at +0x7c, the length at +0x80 and the wrapped byte at +0x84)
// is advanced: the index plus 1 (back to 0, with the byte set to 1, when not below the length, signed), the temp stored at that index and cleared. For 0x1b with the
// object's assigned staff member (the word at +0x5e) as the word at +8 of the message, that word and the dword at +0x60 are cleared (a debug print is left out). Returns
// 0 in those two cases, else the message number.
u32 __fastcall Object_HandleDayMessage(u8 *object, void *, u8 *message);

}  // extern "C"
