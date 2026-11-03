// The sound mixer's thread: see mixerthread.h.
#include "mixerthread.h"

#include <cstdint>

#include "../files/file.h"
#include "soundvoice.h"
#include "vcall.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

const MixerThreadEnv kGameEnv = {Time_GetMs, SoundVoice_UpdateAudible};
const MixerThreadEnv *g_env = &kGameEnv;

u32 take_signal(u8 *mixer) { return (*reinterpret_cast<u32(__stdcall *const *)(u8 *, u32)>((uintptr_t)0x006fd200))(mixer + 0x38, 0); }

void update_voices(u8 *mixer, u32 now) {
    for (u32 i = 0; i < at<u32>(mixer, 0x4c); ++i) {
        u8 *const voice = at<u8 **>(mixer, 0x40)[i];
        if (!voice) continue;
        if (g_env->audible(voice)) {
            u32 out;   // (not read after)
            vcall<void>(voice, 0x14, &out, now);
        }
        u8 *const owned = at<u8 *>(voice, 0x28);
        at<u32>(voice, 0x28) = 0;
        vcall<void>(owned, 0);
        at<u8 **>(mixer, 0x40)[i] = nullptr;
    }
}

}  // namespace

void MixerThread_SetEnv(const MixerThreadEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

void __fastcall SoundMixer_ThreadLoop(u8 *mixer) {
    for (;;) {
        do {
            (*reinterpret_cast<void(__stdcall *const *)(u32)>((uintptr_t)0x006fd1d4))(0x28);   // Sleep
        } while (take_signal(mixer) != 1);
        do {
            if (mixer[0x35]) {
                mixer[0x34] = 1;
                return;
            }
            update_voices(mixer, g_env->now());
        } while (take_signal(mixer) == 1);
    }
}
}
