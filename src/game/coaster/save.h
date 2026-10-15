// Saving the coasters: reimplementation of Coasters_Save (0x437300), which writes every
// link (0x790fe0, next +0x18) with its pieces (+0x54, next +0x64) and its type's (+4) cars
// (+0x128) to a stream.
#pragma once

#include "../../../re/types.h"

struct Stream;

extern "C" {

// 0x437300: writes the link count, 0x790b04, 0x790b00 and 0x791ce0 (4 bytes each), then per
// link a 0x20-byte record (its flags remapped, a count of its pieces whose part (+0x100) lacks
// flag 0x10 at +4, and fields from +8 to +0x140), and per piece: when its +0 is 2, its word
// count (+0xb0) and words (each through a pointer from +0xb4); then, unless its part has 0x10,
// a 0x22-byte record and the words again. Then for each of the type's +0x2e8 cars (0x80 bytes
// from +0x128) a 0x1b-byte record and, for each of its +4 -> +4 seats (0x60 bytes from +0x28),
// a 0xb-byte record and its two arrays (+0x28 and +0x2c, +0x30 and +0x34 dwords). Returns 1.
// sic: a failed write never stops it, each being checked for a result below 0, which a 0 or 1
// result never is.
u32 __cdecl Coasters_Save(Stream *stream);
}
