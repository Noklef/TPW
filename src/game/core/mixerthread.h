// The sound mixer's thread (name invented, unconfirmed). Reimplementation of 0x6b63a0.
#pragma once

#include "../../../re/types.h"

// The calls (the game's by default; the bench's recorders).
struct MixerThreadEnv {
    u32(__cdecl *now)();                  // Time_GetMs
    u8(__fastcall *audible)(u8 *voice);   // SoundVoice_UpdateAudible
};
void MixerThread_SetEnv(const MixerThreadEnv *env);

extern "C" {

// 0x6b63a0 (thiscall; pops nothing): every 40 ms the import 0x6fd200 takes the signal +0x38 (leaving 0). While it was 1: with the stop byte +0x35 set, byte +0x34 = 1 and the end; otherwise every voice in the array +0x40 (+0x4c entries): when audible, its method 0x14 (out, now);
// the object +0x28's method 0 and both its pointers cleared.
void __fastcall SoundMixer_ThreadLoop(u8 *mixer);
}
