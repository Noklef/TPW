// The sound names' table (the singleton at 0xfb1f28, made by 0x6beb20 behind the guard 0x6bead0; the class name is invented): +0x3c and
// +0x4c a deque's begin and end iterators ({block start, block end, current, map node}; blocks of 0x400 pointers), each element an object
// whose String at +0xc (length +0x10, text +0x14) is a name; +0x64 a count. Reimplementation of 0x6c0dc0; the name is unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x6c0dc0 (thiscall (out, stream), pops 8; called from 0x6b6cb2): the table saved to a stream (vtable: +0xc write (buffer, count), +0x10
// seek (offset, origin), +0x18 tell): the position, the count (+0x64) written twice, that many zero dwords, then for each element a byte
// (its name's length + 1, 0 for a null element) and, when that byte is not 0, that many bytes of the name's text; then the size from the
// first position less 8 written there (seek to it, write, seek back to the end). *out = 0. Returns out.
u32 *__fastcall SoundNames_Save(u8 *self, void *, u32 *out, u8 *stream);

}  // extern "C"
