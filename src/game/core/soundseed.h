// The game's random number's seed. Reimplementation of 0x6bb940; the name is invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

// The call (the port by default; the bench's recorder).
struct SoundSeedEnv {
    u32(__cdecl *now)();   // Time_GetMs
};

void SoundSeed_SetEnv(const SoundSeedEnv *env);

// 0x6bb940 (reached from the static initialiser 0x6bb930): the random number 0xfb1f20 (soundvoice.h) seeded from the time.
void __cdecl SoundRandom_Seed();
