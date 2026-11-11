// A sound's start through the engine and mixer: see soundtrigger.h.
#include "soundtrigger.h"

#include <cstdint>

#include "vcall.h"
#include "imports.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename F> F game(u32 address) { return reinterpret_cast<F>((uintptr_t)address); }

const SoundTriggerEnv kGameEnv = {
    game<u32(__fastcall *)(u8 *, void *, u8 *, u8 *, u8 *)>(0x006b6650),
    game<u8 *(__fastcall *)(u8 *, void *, u32 *)>(0x006b68d0),
    game<SoundVoiceMaker>(0x006be750),
    game<SoundVoiceMaker>(0x006be680),
    game<SoundVoiceMaker>(0x006be610),
    game<SoundVoiceMaker>(0x006be330),
    game<SoundVoiceMaker>(0x006be090),
    game<SoundVoiceMaker>(0x006bdd30),
    game<SoundVoiceMaker>(0x006bdc60),
    game<SoundVoiceMaker>(0x006bd7b0),
    game<u8 *(__cdecl *)()>(0x006bead0),
    game<void(__fastcall *)(u8 *, void *, s32)>(0x006b9b30),
    game<u8 *(__fastcall *)(u8 **, void *, u8 *, u8 *)>(0x006c5870),
    game<u8 *(__fastcall *)(u8 *, void *, u32)>(0x006b6830),
};
const SoundTriggerEnv *g_env = &kGameEnv;

SoundVoiceMaker pick_maker(const u8 *definition) {
    const u16 flags = at<u16>(definition, 0x10);
    if (!(flags & 4)) return g_env->make70a3e8;
    if (flags & 0x10) return g_env->makeSequence;
    if (flags & 2) {
        if (flags & 0x400) return g_env->makeSwitch;
        return flags & 0x100 ? g_env->make70a878 : g_env->make70a338;
    }
    return flags & 0x400 ? g_env->make70a2e0 : g_env->make70a390;
}

// Every channel the group's byte +8 asks for allowed by the player's bytes +0x44 up.
bool channels_allowed(u8 wanted, const u8 *player) {
    for (u32 i = 0; i < 4; ++i) {
        if (wanted >> i & 1 && !at<u8>(player, 0x44 + i)) return false;
    }
    return true;
}

}  // namespace

void SoundTrigger_SetEnv(const SoundTriggerEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

u32 __fastcall SoundMixer_AddVoiceSignal(u8 *mixer, void *, u8 *owner, u8 *definition, u8 *sound) {
    if (!g_env->addVoice(mixer, nullptr, owner, definition, sound)) return 0;
    return (*reinterpret_cast<u32(__stdcall *const *)(u8 *, u32)>((uintptr_t)IMPORT(Kernel32_InterlockedExchange)))(mixer + 0x38, 1);
}

u32 __fastcall SoundMixer_Start(u8 *mixer, void *, u8 *owner, u8 *definition, u8 *sound) {
    u8 *const section = mixer + 4;
    (*reinterpret_cast<void(__stdcall *const *)(u8 *)>((uintptr_t)IMPORT(Kernel32_EnterCriticalSection)))(section);
    u32 taken = 0;
    u8 *const voice = g_env->takeVoice(mixer, nullptr, &taken);
    if (voice) {
        const SoundVoiceMaker make = vcall<u32>(sound, 0) == 1 ? g_env->makeOne : pick_maker(definition);
        make(owner, voice, definition, sound);
    }
    (*reinterpret_cast<void(__stdcall *const *)(u8 *)>((uintptr_t)IMPORT(Kernel32_LeaveCriticalSection)))(section);
    return taken;
}

u32 __fastcall SoundEngine_Play(u8 *engine, void *, u32 id, u8 *sound) {
    u8 *const mixer = at<u8 *>(engine, 0x20);
    if (!mixer || !at<u32>(engine, 0x24)) return id;
    u8 *const category = g_env->category();
    const s32 steps = vcall<s32>(sound, 4);
    u32 iterator[4];
    for (u32 i = 0; i < 4; ++i) iterator[i] = at<u32>(category, 0x3c + 4 * i);
    g_env->advance(reinterpret_cast<u8 *>(iterator), nullptr, steps);
    u8 *const owner = *reinterpret_cast<u8 **>((uintptr_t)iterator[2]);
    if (!owner || !at<u32>(owner, 8)) return id;
    u8 *const group = at<u8 *>(owner, 8);
    u8 *const definition = g_env->find(&sound, nullptr, group, sound);   // (`this` the original's argument slot, unused)
    if (!definition) return id;
    if (!(at<u16>(definition, 0x10) & 0xc) && !channels_allowed(at<u8>(group, 8), at<u8 *>(engine, 0x24))) return id;
    u8 *const playing = g_env->findVoice(mixer, nullptr, id);
    if (playing) {
        const u32 flags = at<u32>(playing, 0x30);
        if (!(flags & 0x10)) {
            if (at<u16>(playing, 0x22) >= at<u32>(definition, 0xc)) return id;
            at<u32>(playing, 0x30) = flags | 0x100;
        }
    }
    if (!vcall<u32>(at<u8 *>(engine, 0x24), 0x30, sound, definition)) return id;
    if (at<u8>(definition, 0x10) & 1) {
        SoundMixer_AddVoiceSignal(mixer, nullptr, owner, definition, sound);
        return id;
    }
    return SoundMixer_Start(mixer, nullptr, owner, definition, sound);
}
}
