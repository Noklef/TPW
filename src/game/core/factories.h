// Allocating factories: an object of a class allocated from the game heap and constructed, with the result code the original hands back through COM-style out parameters.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x617ff0 (cdecl; result code out, object out): a null object pointer gives 0x80004003; otherwise a 0x18-byte object is allocated and constructed (Class703770_Ctor, then +8 cleared
// and the vtable 0x7037c0) and stored, with result 0, or the result is 0xcffe0100 when the allocation fails. Returns the result pointer.
u32 *__cdecl Class7037c0_Create(u32 *result, u8 **object);

// 0x617970 (cdecl; result code out, object out): as Class7037c0_Create for a 0x14-byte object (Class703770_Ctor, then +8 and +0xc cleared and the vtable 0x703750). Returns the result pointer.
u32 *__cdecl Class703750_Create(u32 *result, u8 **object);
}  // extern "C"
