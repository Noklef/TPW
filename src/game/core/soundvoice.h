// A sound mixer's voice (an entry of the mixer's voice pages, 0x50 bytes; soundmixer; the class name is invented): +0xc an info block,
// +0x24 its sound, +0x2c its id, +0x30 flags, +0x48 a pending word, +0x4c a parameter block (0x20 bytes: a byte, then dwords). The sound
// system [0xfb1f24] takes settings for a sound as stack functors (vtable, sound, data). Reimplementation of 0x6b7150 and 0x6bc900; the
// names are unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x6b7150 (thiscall): with no parameter block at +0x4c, one of 0x20 bytes (Mem_Alloc; +0 the byte and +4-+0x18 cleared, +0x1c left)
// stored there, or 0 when the allocation fails.
void __fastcall SoundVoice_EnsureParams(u8 *voice);

// 0x6bc900 (thiscall (out, value), pops 8): with a sound at +0x24, a stack record (0x9c bytes: the fields the sound system reads
// cleared, +0x80 / +0x84 / +0x8c 0.25 / 0.5 / 1.0, SetThenFlag28_14 (value)) handed to the sound system's slot 4 (out, a stack
// functor: vtable 0x70a850, the sound, the record). Without one, the value's low word kept at +0x48, flag 0x8000 set and *out = -1.
// Returns out.
u32 *__fastcall SoundVoice_SetPending(u8 *voice, void *, u32 *out, u32 value);

}  // extern "C"
