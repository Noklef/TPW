// A movie's life: see movielife.h.
#include "movielife.h"

#include <cstdint>
#include <cstring>
#include <initializer_list>

#include "../core/crt.h"
#include "moviefile.h"
#include "moviesound.h"
#include "moviesurface.h"
#include "../core/imports.h"
#include "../core/addrcalls.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T game(u32 address) { return GameAddress<T>(address); }
template <typename F> F import(u32 slot) { return *reinterpret_cast<F const *>((uintptr_t)slot); }

u32 exchange(void *place, u32 value) { return import<u32(__stdcall *)(void *, u32)>(IMPORT(Kernel32_InterlockedExchange))(place, value); }
void increment(void *place) { import<u32(__stdcall *)(void *)>(IMPORT(Kernel32_InterlockedIncrement))(place); }
void decrement(void *place) { import<u32(__stdcall *)(void *)>(IMPORT(Kernel32_InterlockedDecrement))(place); }
void sleep(u32 ms) { import<void(__stdcall *)(u32)>(IMPORT(Kernel32_Sleep))(ms); }
u32 timeNow() { return import<u32(__stdcall *)()>(IMPORT(Kernel32_GetTickCount))(); }

const MovieLifeEnv kGameEnv = {game<void *(__cdecl *)(u32, u32)>(0x0067fcb0),
                               &MovieFile_Open,
                               game<void(__fastcall *)(u8 *)>(0x00670860),
                               game<void(__fastcall *)(u8 *)>(0x00672850),
                               game<void(__fastcall *)(u8 *)>(0x006700d0),
                               game<u32(__cdecl *)(u32, u32, u8 *)>(0x0067e050),
                               game<void(__cdecl *)()>(0x0067e180),
                               game<u32(__fastcall *)(u8 *)>(0x006727c0),
                               game<void(__fastcall *)(u8 *)>(0x0066f380),
                               game<void(__fastcall *)(u8 *)>(0x00672a00),
                               game<void(__fastcall *)(u8 *, u8 *)>(0x00670c20),
                               game<void(__fastcall *)(u8 *)>(0x0066f600),
                               game<void(__fastcall *)(u8 *)>(0x0066fb90),
                               game<void(__fastcall *)(u8 *)>(0x00672800),
                               game<void(__fastcall *)(u8 *)>(0x00672bf0)};
const MovieLifeEnv *g_env = &kGameEnv;

u8 *settingsOf(u8 *player) { return at<u8 *>(player, 0); }
void *state(u8 *player) { return settingsOf(player) + 0x9c; }

// A COM object's method (the object first on the stack).
template <typename... A> void com(void *object, u32 slot, A... args) {
    using Method = u32(__stdcall *)(void *, A...);
    (*reinterpret_cast<Method *>(*reinterpret_cast<u8 **>(object) + slot))(object, args...);
}

void freeAndClear(u8 *player, u32 offset) {
    if (at<void *>(player, offset)) {
        Crt().free(at<void *>(player, offset));
        at<void *>(player, offset) = nullptr;
    }
}

// The settings' object at this offset released and cleared.
void release(u8 *player, u32 offset) {
    com(at<void *>(settingsOf(player), offset), 8);
    at<void *>(settingsOf(player), offset) = nullptr;
}

// A sound's frame clock: the player's sound time.
s32 played(u8 *player) { return at<s32>(player, 0x394) - at<s32>(player, 0x3a4); }

void playFrames(u8 *player);

}  // namespace

void MovieLife_SetEnv(const MovieLifeEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

u8 *__cdecl MovieLife_Create(const char *file, u32 value, u8 *settings) {
    u8 *const player = static_cast<u8 *>(g_env->calloc(1, 0x670));
    if (!player) return nullptr;
    std::memset(player, 0, 0x670);
    at<u8 *>(player, 0) = settings;
    at<u32>(settings, 0xa0) = value;
    if (g_env->open(file, player)) {
        Crt().free(player);
        return nullptr;
    }
    g_env->readHeader(player);
    at<u8 *>(settings, 0) = player;
    g_env->soundStart(player);
    g_env->bufferStart(player);
    exchange(settingsOf(player) + 0x9c, 1);
    return settings;
}

u32 __cdecl MovieLife_Destroy(u8 *settings) {
    u8 *const player = at<u8 *>(settings, 0);
    at<s32>(settingsOf(player), 0x90) = at<s32>(player, 0x61c) - at<s32>(player, 0x618);
    MovieFile_Close(player);
    MovieSound_ReleaseAll(player, nullptr);
    MovieLife_ReleaseSurfaces(player);
    exchange(state(player), 7);
    exchange(settingsOf(player), 0);
    Crt().free(player);
    return 0;
}

u32 __cdecl MovieLife_Start(u8 *settings) {
    if (!(at<u32>(settings, 4) & 0x10000)) {
        MovieLife_ThreadMain(settings);
        return 1;
    }
    const u32 thread = g_env->beginThread(0x0066f050, 0, settings);
    at<u32>(settings, 0xac) = thread;
    if (thread == 0xffffffff) return thread;
    void *const place = settings + 0x9c;
    while (exchange(place, 2) != 3) sleep(0x14);
    exchange(place, 3);
    return 0;
}

void __cdecl MovieLife_ThreadMain(u8 *settings) {
    u8 *const player = at<u8 *>(settings, 0);
    const u32 threaded = at<u32>(settingsOf(player), 4) & 0x10000;
    MovieLife_Run(settings);
    if (at<u32>(player, 0x3e0)) MovieLife_Destroy(settings);
    if (threaded) g_env->endThread();
}

void __fastcall MovieLife_Run(u8 *settings) {
    u8 *const player = at<u8 *>(settings, 0);
    if ((at<u32>(settingsOf(player), 4) & 0x30000) == 0x30000) {
        const u32 priority = at<u32>(settingsOf(player), 0xa8);
        import<void(__stdcall *)(u32, u32)>(IMPORT(Kernel32_SetThreadPriority))(import<u32(__stdcall *)()>(IMPORT(Kernel32_GetCurrentThread))(), priority);
    }
    if (!g_env->startThread(player)) playFrames(player);   // (0x66f4d0 before it and after does nothing)
    g_env->releaseStream(player);
    exchange(state(player), 6);
}

}  // extern "C"

namespace {

void playFrames(u8 *player) {
    at<u32>(settingsOf(player), 0x98) = 0;
    exchange(state(player), 2);
    at<u32>(player, 0x618) = timeNow();
    for (;;) {
        u32 old = exchange(state(player), 3);
        u8 *const entry = player + 0x1e8 + (at<s32>(player, 0x37c) % 24) * 0x10;
        if (old == 4) {
            exchange(state(player), 4);
            do {
                sleep(0x14);
                old = exchange(state(player), 4);
            } while (old == 4);
            exchange(state(player), old);
        }
        if (old == 5) break;
        if (old != 3) continue;
        sleep(0);
        g_env->readAhead(player);
        if (!at<u32>(player, 0x368) && at<u32>(settingsOf(player), 0x84)) {
            g_env->createStream(player);
            at<u32>(player, 0x394) = 0;
        }
        if (at<u32>(entry, 0)) {
            if (at<s32>(settingsOf(player), 0x98) <= 2) {
                g_env->showFrame(player, entry);
                g_env->present(player);
            } else {
                const u32 kind = at<u32>(entry, 0xc);
                if (kind == 2 || kind == 1 || kind == 6 || kind == 5) g_env->showFrame(player, entry);
            }
            u8 *const s = settingsOf(player);
            if ((at<u32>(s, 4) & 0x40000) && at<void *>(s, 0xb0) && at<u32(__cdecl *)()>(s, 0xb0)()) break;
            at<u32>(entry, 0) = 0;
            at<u32>(entry, 4) = 0;
            increment(player + 0x37c);
            decrement(player + 0x384);
            g_env->swapFields(player);
        }
        increment(settingsOf(player) + 0x94);
        if (!at<u32>(player, 0x384)) {
            u8 *const s = settingsOf(player);
            if (!at<u32>(s, 0x84) || !(at<u32>(s, 4) & 0x800000)) break;
            if (!at<u32>(player, 0x388)) {
                if (!at<void *>(s, 0x5c)) break;
                u32 status;
                com(at<void *>(s, 0x5c), 0x24, &status);
                if (status & 4) com(at<void *>(settingsOf(player), 0x5c), 0x30, 0u, 0u, 0u);
                if (!(status & 1)) break;
            }
        }
        if (!at<u32>(settingsOf(player), 0x84)) continue;
        s32 time = played(player);
        const s32 wanted = at<s32>(settingsOf(player), 0x94);
        for (s32 turn = 0; wanted - time >= 1;) {
            sleep(0);
            sleep(2);
            time = played(player);
            if (turn++ > 0x96) break;
        }
        at<s32>(settingsOf(player), 0x98) = time - wanted;
        if (at<s32>(settingsOf(player), 0x98) < 0) at<s32>(settingsOf(player), 0x98) = 0;
    }
    at<u32>(player, 0x61c) = timeNow();
    g_env->stopThread(player);
}

}  // namespace

extern "C" {

u8 *__cdecl MovieLife_OpenAndStart(const char *file, u32 value, u8 *settings) {
    u8 *const made = MovieLife_Create(file, value, settings);
    if (!made) return nullptr;
    at<u32>(at<u8 *>(made, 0), 0x3e0) = 1;
    return MovieLife_Start(made) ? nullptr : made;
}

void __fastcall MovieLife_ReleaseSurfaces(u8 *player) {
    if (!at<u32>(player, 0x3c)) return;
    for (u32 offset : {0x658u, 0x65cu, 0x660u, 0x648u, 0x64cu, 0x654u, 0x650u}) freeAndClear(player, offset);
    if (at<u32>(player, 0x4c) == 2) {
        MovieSurface_UpdateOverlay(player, 0);
        sleep(0x14);
    }
    u8 *const s = settingsOf(player);
    if (at<void *>(s, 0x24) && !(at<u32>(s, 4) & 0x4000000)) release(player, 0x24);
    if (at<void *>(settingsOf(player), 0x20) && !(at<u32>(settingsOf(player), 4) & 0x800)) release(player, 0x20);
    if (at<void *>(settingsOf(player), 0x1c) && !(at<u32>(settingsOf(player), 4) & 0x800)) release(player, 0x1c);
    if (at<u32>(player, 0x3e4)) {
        release(player, 0x10);
        at<u32>(player, 0x3e4) = 0;
    }
    at<u32>(player, 0x3c) = 0;
}
}
