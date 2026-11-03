// The sound threads' waits: see soundwait.h.
#include "soundwait.h"

#include <cstdint>

#include "channelvoice.h"
#include "crt.h"
#include "soundworker.h"
#include "vcall.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename T> T &at(u8 *p, u32 offset) { return *reinterpret_cast<T *>(p + offset); }

using WaitForMultipleObjectsFn = u32(__stdcall *)(u32 count, const void *handles, u32 all, u32 milliseconds);

const SoundWaitEnv kGameEnv = {ChannelVoice_Refill, SoundWorker_RefillVoice, CRT_atexit};
const SoundWaitEnv *g_env = &kGameEnv;

}  // namespace

void SoundWait_SetEnv(const SoundWaitEnv *env) { g_env = env ? env : &kGameEnv; }

void __fastcall ChannelVoices_WaitLoop(u8 *pool) {
    volatile u32 &flags = at<volatile u32>(pool, 0x14);
    flags = flags | 2;
    if (!(flags & 1)) {
        const WaitForMultipleObjectsFn wait = global<WaitForMultipleObjectsFn>(0x006fd160);
        do {
            const u32 index = wait(at<u32>(pool, 8), at<void *>(pool, 0x10), 0, 0xffffffffu);
            if (index == at<u32>(pool, 0xc)) break;
            g_env->refillVoice(at<u8 *>(pool, 4) + index * 0x8c);
        } while (!(flags & 1));
    }
    flags = flags & ~2u;
}

void __fastcall SoundSource_WaitLoop(u8 *source) {
    volatile u32 &flags = at<volatile u32>(source, 8);
    flags = flags | 0x80;
    if (flags & 0x40) {
        const WaitForMultipleObjectsFn wait = global<WaitForMultipleObjectsFn>(0x006fd160);
        do {
            const u32 index = wait(at<u32>(source, 0x30), at<void *>(source, 0x2c), 0, 0xffffffffu);
            if (index == at<u32>(source, 0x30) - 1) break;
            g_env->refillPosition(source, nullptr, index);
        } while (flags & 0x40);
    }
    flags = flags & ~0x80u;
}

void __cdecl SoundHandshake_AtExit() {}

void __fastcall SoundHandshake_Wait(u8 *handshake) {
    at<volatile u8>(handshake, 4) = 1;
    while (!at<volatile u8>(handshake, 5)) {
    }
    at<u32>(handshake, 0) = 0;
}

u8 *__cdecl SoundHandshake_Instance() {
    u8 *const instance = &global<u8>(0x00fb1f10);
    u8 &guard = global<u8>(0x00fb1f08);
    if (!(guard & 1)) {
        guard = guard | 1;
        at<u32>(instance, 0) = 0;
        at<u8>(instance, 4) = 0;
        at<u8>(instance, 5) = 0;
        g_env->atexit(reinterpret_cast<void *>((uintptr_t)0x006b9ec0));   // (SoundHandshake_AtExit, by its hook)
    }
    return instance;
}

void __fastcall SoundEngine_Dtor(u8 *engine) {
    at<u32>(engine, 0) = 0x0070a2a0;
    SoundHandshake_Wait(SoundHandshake_Instance());
    at<u32>(engine, 0) = 0x0070a588;
    if (u8 *const inner = at<u8 *>(engine, 0x24)) vcall<void>(inner, 0);
    if (u8 *const inner = at<u8 *>(engine, 0x20)) vcall<void>(inner, 0, 1u);
}
