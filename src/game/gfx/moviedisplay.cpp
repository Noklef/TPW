// A movie's display made ready: see moviedisplay.h.
#include "moviedisplay.h"

#include <cstdint>
#include <cstring>

#include "moviepixels.h"
#include "moviesurface.h"
#include "tgqvideo.h"
#include "../core/addrcalls.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T game(u32 address) { return GameAddress<T>(address); }

const MovieDisplayEnv kGameEnv = {&MovieSurface_PickMode, &MovieSurface_ReadSize, &MoviePixels_ReadFormat, game<void *(__cdecl *)(u32)>(0x0067dcd0), &TgqVideo_SetupTables, &MoviePixels_BuildTables};
const MovieDisplayEnv *g_env = &kGameEnv;

u8 *settingsOf(u8 *player) { return at<u8 *>(player, 0); }

// A COM object's method (the object first on the stack).
template <typename... A> void com(void *object, u32 slot, A... args) {
    using Method = u32(__stdcall *)(void *, A...);
    (*reinterpret_cast<Method *>(*reinterpret_cast<u8 **>(object) + slot))(object, args...);
}

}  // namespace

void MovieDisplay_SetEnv(const MovieDisplayEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" void __fastcall MovieDisplay_Prepare(u8 *player) {
    u8 *s = settingsOf(player);
    if (at<u32>(s, 0x88)) return;
    if (!at<void *>(s, 0x10) && at<void *>(s, 0xc)) {
        at<u32>(player, 0x3e4) = 1;
        com(at<void *>(s, 0xc), 0, 0x7071f0u, s + 0x10);
    }
    at<u32>(player, 0x4c) = g_env->pickMode(player, nullptr);
    at<u32>(settingsOf(player), 0x1c) = 0;
    at<u32>(settingsOf(player), 0x20) = 0;
    if (!(at<u32>(settingsOf(player), 4) & 0x800)) {
        g_env->readSize(player, nullptr);
        s = settingsOf(player);
        g_env->readFormat(at<void *>(s, 0x10), at<void *>(s, 0x14), reinterpret_cast<u32 *>(s + 0x2c));
        s = settingsOf(player);
        if (at<u32>(s, 0x48) == 8 && !(at<u32>(s, 4) & 0x4000000)) {
            at<void *>(settingsOf(player), 0x28) = g_env->malloc(0x400);
            s = settingsOf(player);
            com(at<void *>(s, 0x10), 0x14, 0x44u, at<void *>(s, 0x28), s + 0x24, 0u);
            std::memset(at<void *>(settingsOf(player), 0x28), 0, 0x400);
            s = settingsOf(player);
            com(at<void *>(s, 0x14), 0x7c, at<void *>(s, 0x24));
        }
    }
    s = settingsOf(player);
    if (!(at<u32>(s, 4) & 0x100000)) at<u32>(s, 0x78) = 100;
    g_env->setupTables(player);
    g_env->buildTables(reinterpret_cast<const u32 *>(settingsOf(player) + 0x2c), player);
    for (s32 k = -0x200; k < 0x200; ++k) {
        const s32 clamped = k < -0x80 ? -0x80 : k > 0x7f ? 0x7f : k;
        gMoviedisplay_fb7f30[(k - 0x180) & 0x3ff] = static_cast<u32>(clamped + 0x80);
    }
    at<u32>(player, 0x3c) = 1;
}
