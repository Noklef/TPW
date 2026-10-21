// The sound object's channel (its +0x24; vtable 0x70a5d8, made by 0x6b8ed0 / 0x6b9c00; the class name is invented): +0x28 flags
// (0x400: its voices are run through the mixer), +0x2c a float, +0x44 four bytes, +0x48 an array of floats. Settings reach it as stack
// functors (a vtable whose slot 0 is thiscall (out, channel), pops 8, and the functor's data). Reimplementation of 0x6b8b40; the names
// are unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x6b8b40 (thiscall (out, functor), pops 8): the functor's slot 0 (out, channel). Returns out.
u32 *__fastcall SoundChannel_Apply(u8 *channel, void *, u32 *out, u8 *functor);

}  // extern "C"
