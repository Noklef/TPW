// A movie's start, its end and its stop: see moviestart.h.
#include "moviestart.h"

#include <cstdint>
#include <cstring>

#include "../core/vcall.h"
#include "../core/accessors.h"
#include "../render/screen.h"
#include "../render/graphicssettings.h"
#include "../render/d3dsetup.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename F> F game(u32 address) { return reinterpret_cast<F>((uintptr_t)address); }
u8 *address(u32 a) { return reinterpret_cast<u8 *>((uintptr_t)a); }

const MovieStartEnv kGameEnv = {
    game<u32(__cdecl *)()>(0x0051b770),
    game<u32(__cdecl *)()>(0x0051c6f0),
    game<u32(__fastcall *)(u8 *)>(0x00564210),
    game<u8 *(__cdecl *)(u32, u32, u8 *)>(0x0066f4e0),
    game<u32(__cdecl *)(u8 *, s32)>(0x0066f520),
    game<u32(__cdecl *)(u8 *)>(0x0066f540),
    game<u32(__cdecl *)(u8 *)>(0x0066f580),
};
const MovieStartEnv *g_env = &kGameEnv;

u32 &allowed() { return gAccessors_802bc0; }
u32 &on() { return gMoviestart_802bbc; }
u8 *&player() { return gMoviestart_802bb4; }
u8 *&front() { return gMovieStart_Settings_8023f0.front; }
u8 *&back() { return gMovieStart_Settings_8023f0.back; }
u32 &flags() { return gMovieStart_Settings_8023f0.flags; }

u32 query(u8 *object, u32 iid, void *out) { return comcall<u32>(object, 0, address(iid), out); }

void release_interfaces() {
    if (back()) comcall<u32>(back(), 8);
    if (front()) comcall<u32>(front(), 8);
    back() = nullptr;
    front() = nullptr;
}

void clear_back(u8 *surface, u32 *fx) {
    while (comcall<u32>(surface, 0x14, nullptr, nullptr, nullptr, 0x1000400u, fx) != 0) {
    }
    g_env->flip(global<u8 *>((u32)(uintptr_t)&gD3dsetup_879a20 + 0x4));
}

void stop() {
    if (player() && g_env->playing(player())) g_env->stop(player());
    release_interfaces();
    player() = nullptr;
    on() = 0;
}

}  // namespace

void MovieStart_SetEnv(const MovieStartEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

u32 __cdecl Movie_Start(u32 file, u32 mode, s32 x, s32 y, u32 overlay) {
    const u32 sound = g_env->soundWord();
    if (!allowed() || !file || on()) return 0;
    gMovie_Start_802bb0 = 0;
    MovieSettings &settings = gMovieStart_Settings_8023f0;
    memset(&settings, 0, sizeof settings);
    gMovieStart_8023e8 = 0;
    u8 *const renderer = global<u8 *>((u32)(uintptr_t)&gD3dsetup_879a20 + 0x4);
    settings.renderer = at<u32>(renderer, 0);
    u8 *back_surface, *front_surface;
    if (query(at<u8 *>(renderer, 0x14), 0x00707210, &back_surface)) return 0;
    if (query(at<u8 *>(global<u8 *>((u32)(uintptr_t)&gD3dsetup_879a20 + 0x4), 0x10), 0x00707210, &front_surface)) return 0;
    if (query(front_surface, 0x00707220, &front())) return 0;
    if (query(back_surface, 0x00707220, &back())) return 0;
    settings.field70 = gScreen_7a1968;
    settings.fieldA8 = 2;
    settings.kind = 1;
    settings.field74 = gScreen_7a1930 * 352 / 480;
    u32 f = reinterpret_cast<u8 &>(gGraphicssettings_7a1a8c) & 1 ? 0x0b030180 : 0x03030180;
    if (x == -1) {
        f |= 4;
    } else {
        f |= 1;
        settings.x = x;
    }
    if (y == -1) {
        f |= 8;
    } else {
        f |= 2;
        settings.y = y;
    }
    if (overlay) {
        f |= 0x200;
        settings.kind = 7;
    }
    if (sound) f |= 0x200000;
    flags() = f;
    if (g_env->soundOn()) flags() |= 0x800000;
    settings.sound = sound;
    u32 fx[0x19] = {0x64};
    if (back_surface) {
        clear_back(back_surface, fx);
        clear_back(back_surface, fx);
    }
    u32 result = 0;
    player() = g_env->open(file, mode, reinterpret_cast<u8 *>(&settings));
    if (player()) {
        result = 1;
    } else {
        release_interfaces();
    }
    on() = 1;
    if (player()) g_env->setVolume(player(), (s32)(gAccessors_802ba8 * 10000u) / 1023 - 10000);
    return result;
}

u32 __cdecl Movie_Finished() {
    if (!on() || !allowed()) return 1;
    game<void(__stdcall *const *)(u32)>(0x006fd1d4)[0](10);
    const u32 finished = !player() || g_env->playing(player()) == 0;
    if (finished) on() = 0;
    return finished;
}

void __cdecl Movie_Stop() {
    if (on() && allowed()) stop();
}

void __cdecl Movie_Forbid() {
    if (on() && allowed()) stop();
    allowed() = 0;
}
}
