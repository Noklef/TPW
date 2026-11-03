#include "moviecontrol.h"

#include <cstdint>

#include "moviechunks.h"

namespace {

template <typename T> T &at(u8 *base, u32 offset) { return *reinterpret_cast<T *>(base + offset); }

const MovieControlEnv kGameEnv = {&MovieChunks_Next, reinterpret_cast<u32(__cdecl *)(u32, u32, u8 *)>((uintptr_t)0x0067e050)};
const MovieControlEnv *g_env = &kGameEnv;

u32 exchange(u32 *target, u32 value) { return (*reinterpret_cast<u32(__stdcall *const *)(u32 *, u32)>((uintptr_t)0x006fd200))(target, value); }
void sleep(u32 milliseconds) { (*reinterpret_cast<void(__stdcall *const *)(u32)>((uintptr_t)0x006fd1d4))(milliseconds); }

const s32 kFull = 0x16;

// Chunks until one is not sound, the last's answer kept.
void nextUntilNotSound(u8 *player) {
    s32 answer;
    do {
        answer = g_env->next(player);
        at<s32>(player, 0x3ac) = answer;
    } while (answer == 2);
}

}  // namespace

void MovieControl_SetEnv(const MovieControlEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

void __fastcall MovieControl_ReadAhead(u8 *player) {
    if (at<s32>(player, 0x3ac) == -1) {
        MovieControl_Rewind(player);
        if (at<s32>(player, 0x3ac) == -1) return;
    }
    if (at<s32>(player, 0x388) >= kFull || at<s32>(player, 0x384) >= kFull) return;
    const s32 answer = g_env->next(player);
    at<s32>(player, 0x3b0) = answer;
    u8 *const s = at<u8 *>(player, 0);
    if (answer == -1) {
        MovieControl_Rewind(player);
    } else if (answer == 2) {
        if (at<u32>(s, 0x88) == 0 || at<s32>(player, 0x388) < kFull) nextUntilNotSound(player);
    } else if (answer == 1) {
        if (at<u32>(s, 0x84) == 0) return;
        const s32 after = g_env->next(player);
        at<s32>(player, 0x3ac) = after;
        if (after == 1) {
            at<u32>(s, 0x84) = 0;
        } else if (after == 2) {
            nextUntilNotSound(player);
        }
    }
}

void __fastcall MovieControl_Rewind(u8 *player) {
    u8 *const s = at<u8 *>(player, 0);
    if (at<u32>(s, 4) & 0x8000) {
        sleep(0);
        (*reinterpret_cast<s32(__stdcall *const *)(u32, u32, u32)>((uintptr_t)0x006fd3d8))(at<u32>(s, 0xa4), at<u32>(player, 0x3b8), 0);
        at<s32>(player, 0x3ac) = g_env->next(player);
    } else {
        at<s32>(player, 0x3ac) = -1;
    }
}

u32 __cdecl MovieControl_SetVolume(u8 *settings, u32 volume) {
    u8 *const sound = at<u8 *>(settings, 0x5c);
    if (sound) (*reinterpret_cast<s32(__stdcall **)(u8 *, u32)>(*reinterpret_cast<u8 **>(sound) + 0x3c))(sound, volume);
    return 0;
}

s32 __cdecl MovieControl_Stop(u8 *settings) {
    u32 *const state = &at<u32>(settings, 0x9c);
    const u32 now = *state;
    if (now == 7 || now == 6 || now == 0) return -1;
    u8 *const player = at<u8 *>(settings, 0);
    if (!player) return -1;
    exchange(state, 5);
    const u32 target = at<u32>(player, 0x3e0) ? 7 : 6;
    while (*const_cast<const volatile u32 *>(state) != target) sleep(10);
    return 0;
}

u32 __fastcall MovieControl_StartThread(u8 *player) {
    exchange(&at<u32>(player, 0x3c0), 1);
    const u32 thread = g_env->beginThread(0x006724a0, 0, player);
    at<u32>(player, 0x3c4) = thread;
    (*reinterpret_cast<s32(__stdcall *const *)(u32, u32)>((uintptr_t)0x006fd1fc))(thread, 0xf);
    return at<u32>(player, 0x3c4) == 0;
}

u32 __fastcall MovieControl_StopThread(u8 *player) {
    if (at<u32>(player, 0x3c4) == 0) return 1;
    u32 *const flag = &at<u32>(player, 0x3c0);
    exchange(flag, 0);
    while (exchange(flag, 0) == 0) sleep(10);
    return 0;
}
}
