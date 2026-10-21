// The looping sound stream (vtable 0x70aba8, on the stream filter 0x70a678; the names are invented): a stream over another (its slot 18
// answers the inner stream, +4; the inner's slot 7 reads, 12 seeks, 13 tells), +0x14 the loops left (9999 for ever), +0x1c the loop's start,
// +0x20 its end (Fn_6c6a00 sets them). Reimplementation of 0x6c6a60.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x6c6a60 (thiscall (buffer, count, done, unused), pops 0x10; slot 7 of 0x70aba8): *done = 0, then while it is below the count: the inner
// stream's read (slot 7: buffer + *done, the count left, but no further than the loop's end from the inner's position (slot 13) while
// still looping, a local got and 1); its answer 1 ends it (1). Else, still looping, an inner position at or past the end takes a loop
// (9999 stays): none left ends the looping, else the inner seeks to the start (slot 12). *done += got. The looping ended, slot 3 at the
// end (on either answer). Returns 0, or 1 as said.
u32 __fastcall LoopStream_Read(u8 *stream, void *, u8 *buffer, u32 count, u32 *done, u32 unused);

}  // extern "C"
