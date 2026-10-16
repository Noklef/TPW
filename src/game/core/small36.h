// The block read and write of the wide streambuf: reimplementation of 0x4f0700 and 0x4f07c0. Named by what they do until the class is known.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x4f0700 (thiscall on the streambuf; destination, count, pops 8): copies up to count wide characters from the get area (the pointer behind the pointer at +0x1c, the
// remaining count (31 bits, sign extended) behind the pointer at +0x2c; both advanced) and, when the area is empty, takes one through slot 5 (offset 0x14) until it returns
// 0xffff. Returns the number copied.
u32 __fastcall Wstreambuf_Read(u8 *object, void *, u16 *destination, s32 count);

// 0x4f07c0 (thiscall; source, count, pops 8): the same into the put area (pointer behind +0x20, count behind +0x30), the character of an empty area given to slot 1 (offset 4)
// with the high half of the put pointer above it (sic), until that returns 0xffff. Returns the number written.
u32 __fastcall Wstreambuf_Write(u8 *object, void *, const u16 *source, s32 count);

}  // extern "C"
