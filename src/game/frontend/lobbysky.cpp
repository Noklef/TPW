#include "lobbysky.h"

#include <cstdint>

#include "../core/string.h"
#include "../terrain/landscape.h"

namespace {

template <typename T>
T &at(const void *p, u32 offset) {
    return *reinterpret_cast<T *>(reinterpret_cast<uintptr_t>(p) + offset);
}

template <typename T>
T &global(uintptr_t address) {
    return *reinterpret_cast<T *>(address);
}

constexpr u32 kFloat500 = 0x43fa0000, kFloat100 = 0x42c80000, kFloat180 = 0x43340000, kFloat115 = 0x42e60000, kFloat50 = 0x42480000, kFloat300 = 0x43960000, kFloat1 = 0x3f800000;

const LobbySkyEnv kGameEnv = {reinterpret_cast<void(__fastcall *)(void *, void *, const char *)>((uintptr_t)0x005852b0), reinterpret_cast<void(__fastcall *)(void *, void *, u32, u32, u32)>((uintptr_t)0x00585690),
                              reinterpret_cast<void(__fastcall *)(u8 *, void *, u32, const u32 *, const u32 *, u32)>((uintptr_t)0x005d92e0), reinterpret_cast<void(__fastcall *)(u8 *)>((uintptr_t)0x005d8de0), &Landscape_ResetDomeCentre};
const LobbySkyEnv *g_env = &kGameEnv;

}  // namespace

void LobbySky_SetEnv(const LobbySkyEnv *env) { g_env = env ? env : &kGameEnv; }

void __fastcall LobbySky_Enter(u8 *self) {
    at<u32>(self, 0x34) = global<u32>(0x008bcbc8);
    at<u32>(self, 0x38) = gLobbysky_8bd47c;
    at<u32>(self, 0x3c) = gLobbysky_8bd490;
    at<u32>(self, 0x40) = gLobbysky_8bd494;
    String folder;
    String_CtorStr(&folder, nullptr, reinterpret_cast<const char *>((uintptr_t)0x00774c6c));
    void *const landscape = reinterpret_cast<void *>((uintptr_t)0x008c1828);
    g_env->loadSky(landscape, nullptr, folder.data);
    g_env->turnSky(landscape, nullptr, 0, kFloat180, 0);
    gLobbysky_8bd47c = 0xff44ddff;
    gLobbysky_8bd490 = kFloat50;
    global<u32>(0x008bcbc8) = (global<u32>(0x008bcbc8) & 0xfffeffff & ~0x2000u) | 0x4000000;
    gLobbysky_8bd494 = kFloat300;
    at<u32>(self, 0x8) = kFloat500;
    at<u32>(self, 0xc) = kFloat100;
    at<u32>(self, 0x10) = kFloat500;
    at<u32>(self, 0x14) = kFloat500;
    at<u32>(self, 0x18) = 0;
    at<u32>(self, 0x1c) = kFloat500;
    at<u32>(self, 0x4) = kFloat115;
    gLobbysky_879dd8[0] = kFloat1;
    gLobbysky_879dd8[1] = 0;
    gLobbysky_879dd8[2] = 0;
    gLobbysky_879dd8[3] = 0;
    g_env->light(self, nullptr, 0, nullptr, nullptr, 0);
    at<u32>(self, 0x28) = 0;
    at<u32>(self, 0x2c) = 0;
    at<u32>(self, 0x30) = 0;
    g_env->lookAt(self);
    for (u32 offset = 0x44; offset <= 0x60; offset += 4) at<u32>(self, offset) = 0;
    String_Dtor(&folder);
}

u32 __fastcall LobbySky_Leave(u8 *self) {
    gLobbysky_879dd8[0] = kFloat1;
    gLobbysky_879dd8[1] = 0;
    gLobbysky_879dd8[2] = 0;
    gLobbysky_879dd8[3] = 0;
    g_env->light(self, nullptr, 0, nullptr, nullptr, 0);
    global<u32>(0x008bcbc8) = at<u32>(self, 0x34);
    gLobbysky_8bd47c = at<u32>(self, 0x38);
    gLobbysky_8bd490 = at<u32>(self, 0x3c);
    gLobbysky_8bd494 = at<u32>(self, 0x40);
    return g_env->resetDome(reinterpret_cast<u8 *>((uintptr_t)0x008c1828));
}
