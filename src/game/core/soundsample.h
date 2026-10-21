// The sound samples' shared slots (the sound streams of src/game/core/soundstream that are sources, vtables 0x70a6c8-0x70aec0; the
// names are invented): +0x1c the time (the game's clock, ms) at which a sample whose data has run out has finished playing (0 while it
// still has data), +0x20 the format byte (bit 1 stereo, bit 6 16-bit, bits 3 / 4 the rate, see Obj_SampleRate20). Reimplementation of
// 0x6bb1d0, 0x6c7010 and 0x6c7070.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x6bb1d0 (thiscall (record), pops 4; slot 5): SoundStream_Describe (record).
void __fastcall SoundSample_Describe(u8 *sample, void *, u8 *record);
// 0x6c7010 (thiscall (flag, bytes), pops 8; slot 19): with the flag's low byte 0, the time the bytes left take to play (halved for 16-bit
// and again for stereo, * 1000 / the sample rate (slot 6), unsigned) + 200 ms from now (Time_GetMs) into +0x1c; returns 1. Else 0.
u8 __fastcall SoundSample_ScheduleEnd(u8 *sample, void *, u32 flag, u32 bytes);
// 0x6c7070 (thiscall): 1 when the game's clock (Time_GetMs) has reached +0x1c (unsigned), else 0.
u32 __fastcall SoundSample_Drained(u8 *sample);

}  // extern "C"
