// The movie's sound device and thread: see moviesoundthread.h.
#include "moviesoundthread.h"

#include <cstdint>
#include <cstring>

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T game(u32 address) { return reinterpret_cast<T>((uintptr_t)address); }
template <typename F> F import(u32 slot) { return *reinterpret_cast<F const *>((uintptr_t)slot); }

u32 exchange(void *place, u32 value) { return import<u32(__stdcall *)(void *, u32)>(0x006fd200)(place, value); }
void increment(void *place) { import<u32(__stdcall *)(void *)>(0x006fd0c0)(place); }
void decrement(void *place) { import<u32(__stdcall *)(void *)>(0x006fd12c)(place); }
void sleep(u32 ms) { import<void(__stdcall *)(u32)>(0x006fd1d4)(ms); }

const MovieSoundThreadEnv kGameEnv = {game<void(__cdecl *)()>(0x0067e180), game<u32(__fastcall *)(u8 *)>(0x00672730), game<u32(__fastcall *)(void *)>(0x00672950),
                                      game<u32(__stdcall *)(const void *, void **, void *)>(0x006d76e0)};
const MovieSoundThreadEnv *g_env = &kGameEnv;

u8 *settingsOf(u8 *player) { return at<u8 *>(player, 0); }

// A COM object's method (the object first on the stack).
template <typename... A> u32 com(void *object, u32 slot, A... args) {
    using Method = u32(__stdcall *)(void *, A...);
    return (*reinterpret_cast<Method *>(*reinterpret_cast<u8 **>(object) + slot))(object, args...);
}

struct Spans {
    u8 *first, *second;
    u32 firstBytes, secondBytes;
};

u32 lock(u8 *player, u32 bytes, Spans &spans) {
    return com(at<void *>(settingsOf(player), 0x5c), 0x2c, at<u32>(player, 0x370), bytes, &spans.first, &spans.firstBytes, &spans.second, &spans.secondBytes, 0u);
}

void unlock(u8 *player, const Spans &spans) { com(at<void *>(settingsOf(player), 0x5c), 0x4c, spans.first, spans.firstBytes, spans.second, spans.secondBytes); }

}  // namespace

void MovieSoundThread_SetEnv(const MovieSoundThreadEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

void __fastcall MovieSoundThread_Write(u8 *player, u32 distance) {
    u32 *const block = reinterpret_cast<u32 *>(player + 0x68 + (at<s32>(player, 0x374) % 24) * 0x10);
    Spans spans;
    if (at<u32>(player, 0x388)) {
        if (!at<u32>(player, 0x3cc)) {
            while (lock(player, block[1], spans)) com(at<void *>(settingsOf(player), 0x5c), 0x50);
            const u8 *const data = reinterpret_cast<const u8 *>((uintptr_t)block[0]);
            std::memcpy(spans.first, data, spans.firstBytes);
            if (spans.secondBytes) std::memcpy(spans.second, data + spans.firstBytes, spans.secondBytes);
            unlock(player, spans);
        }
        at<s32>(player, 0x370) = (at<s32>(player, 0x370) + static_cast<s32>(block[1])) % at<s32>(player, 0x36c);
        increment(player + 0x374);
        exchange(player + 0x394, block[2]);
        decrement(player + 0x388);
        block[0] = 0;
        block[1] = 0;
        return;
    }
    if (!at<u32>(player, 0x3cc) && !lock(player, distance, spans)) {
        std::memset(spans.first, 0, spans.firstBytes);
        if (spans.secondBytes) std::memset(spans.second, 0, spans.secondBytes);
        unlock(player, spans);
    }
    at<s32>(player, 0x370) = (static_cast<s32>(distance) + at<s32>(player, 0x370)) % at<s32>(player, 0x36c);
    increment(player + 0x394);
}

void __cdecl MovieSoundThread_Main(u8 *player) {
    sleep(10);
    void *const flag = player + 0x3c0;
    while (exchange(flag, 1)) {
        if (at<u32>(player, 0x368)) {
            const s32 distance = static_cast<s32>(g_env->distance(player));
            if (distance >= at<s32>(player, 0x68 + (at<s32>(player, 0x374) % 24) * 0x10 + 4)) MovieSoundThread_Write(player, static_cast<u32>(distance));
        }
        increment(player + 0x3bc);
        sleep(10);
    }
    g_env->endThread();
}

u32 __fastcall MovieSoundThread_CreateDevice(void **place, u32 window) {
    if (!g_env->create(nullptr, place, nullptr)) {
        if (!com(*place, 0x18, window, 2u)) return 0;
        com(*place, 8);
    }
    *place = nullptr;
    return 1;
}

u32 __fastcall MovieSoundThread_OpenDevice(u8 *player) {
    u8 *s = settingsOf(player);
    if (!(at<u32>(s, 4) & 0x800000)) return 0;
    if (!at<void *>(s, 0x54)) {
        at<u32>(player, 0x3dc) = 1;
        MovieSoundThread_CreateDevice(&at<void *>(s, 0x54), at<u32>(s, 0xa0));
    }
    s = settingsOf(player);
    if (!at<void *>(s, 0x54)) {
        at<u32>(s, 0x58) = 0;
        return 0;
    }
    u32 caps[0x18] = {0x60};
    com(at<void *>(s, 0x54), 0x10, caps);
    at<u32>(player, 0x3d0) = caps[1] >> 5 & 1;
    s = settingsOf(player);
    at<u32>(player, 0x3d4) = (at<u32>(s, 4) & 0x400000) && at<void *>(s, 0x58) ? g_env->bytes(at<void *>(s, 0x58)) : 0x8000;
    return 0;
}
}
