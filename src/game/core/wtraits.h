// The 16-bit character traits the game's wide strings use (the IME at 0x47905e, 0x47aebf,
// 0x47b674): reimplementation of 0x479150.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x479150: moves n characters from src to dst a character at a time, from the end when dst
// lies inside [src, src + n), else from the start; returns dst.
u16 *__cdecl WTraits_Move(u16 *dst, const u16 *src, u32 n);
}
