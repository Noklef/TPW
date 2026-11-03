// The game's random number's seed: see soundseed.h.
#include "soundseed.h"

#include <cstdint>

#include "../files/file.h"

namespace {

const SoundSeedEnv kGameEnv = {Time_GetMs};
const SoundSeedEnv *g_env = &kGameEnv;

}  // namespace

void SoundSeed_SetEnv(const SoundSeedEnv *env) { g_env = env ? env : &kGameEnv; }

void __cdecl SoundRandom_Seed() { *reinterpret_cast<u32 *>((uintptr_t)0x00fb1f20) = g_env->now(); }
