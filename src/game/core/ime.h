// The input method editor support (vtable 0x6fef04; "Microsoft IME 98" is singled out at
// 0x478fc9): reimplementation of its constructor 0x478b60. Four vectors (a one-byte allocator,
// then first / last / end) at +8, +0x18, +0x28 and +0x38 (the last of 0x24-byte entries), +0x48
// a flag, +0x4c / +0x50, +0x54 (-1 when none) and +0x5c a handle the destructor closes.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x478b60: the vectors empty, the rest cleared (+0x54 = -1); returns ime. The allocator bytes
// take an uninitialised local, which is the top byte of ime as pushed (sic).
u8 *__fastcall Ime_Ctor(u8 *ime);
}
