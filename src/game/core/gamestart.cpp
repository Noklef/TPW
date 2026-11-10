#include "gamestart.h"

#include <cstdint>

#include "string.h"
#include "../render/screen.h"
#include "../render/graphicssettings.h"
#include "gamewindow.h"
#include "soundset.h"
#include "../world/camera.h"
#include "gameframe.h"
#include "imports.h"

namespace {

AddrCalls g_calls;

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename T> T ptr(u32 address) { return reinterpret_cast<T>((uintptr_t)address); }

constexpr u32 kClock = 0x00785970, kArgs = 0x008787a0, kLoadScreen = 0x008c4748;
const u32 kDayMovies[8] = {(u32)(uintptr_t)&gGamestart_7650a0, (u32)(uintptr_t)&gGamestart_76508c, (u32)(uintptr_t)&gGamestart_765074, (u32)(uintptr_t)&gGamestart_765060, (u32)(uintptr_t)&gGamestart_76504c, (u32)(uintptr_t)&gGamestart_765034, (u32)(uintptr_t)&gGamestart_765020, (u32)(uintptr_t)&gGamestart_765008};

char *const kPath = gGamestart_8782d0;

void setPath(u32 text) {
    const char *from = ptr<const char *>(text);
    char *to = kPath;
    while ((*to++ = *from++) != 0) {
    }
}

bool lookUp(String *found) {
    String path;
    String_CtorStr(&path, nullptr, kPath);
    const bool ok = (u8)g_calls.Cd(0x005c5110, &path, found) == 1;
    String_Dtor(&path);
    return ok;
}

void play(const String *movie) {
    const u32 height = (u32)(((unsigned long long)(gScreen_7a1930 << 6) * 0x88888889ull) >> 40);
    if (reinterpret_cast<u8 &>(gGraphicssettings_7a1a8c) & 1) {
        g_calls.Cd(0x0051b010, movie->data, gGamewindow_7a130c, 0u, height, 0u);
        return;
    }
    s32 origin[2] = {0, 0};
    global<s32(__stdcall *)(u32, s32 *)>(IMPORT(User32_ScreenToClient))(gGamewindow_7a130c, origin);
    const u32 lowered = (u32)(((unsigned long long)(gScreen_7a1930 << 6) * 0x88888889ull) >> 40);
    g_calls.Cd(0x0051b010, movie->data, gGamewindow_7a130c, 0 - origin[0], lowered - origin[1], 0u);
}

}  // namespace

void GameStart_SetCalls(const AddrCall *table, u32 count) { g_calls.Set(table, count); }

void __cdecl Game_Start() {
    g_calls.Th(0x00402ea0, kClock, 0x20u);
    u32 folders;
    if (global<u32>(0x0078d968))
        folders = global<u32>(0x0079d86c) & 0x800 ? 1 : 0;
    else
        folders = global<u32>(0x00785874) <= 1 ? 1 : 0;
    g_calls.Cd(0x004688b0, folders);
    g_calls.Cd(0x005d55a0);
    const u32 count = g_calls.Th(0x004056f0, kArgs);
    if (!g_calls.Cd(0x0040f000, g_calls.Th(0x004056e0, kArgs), count)) {
        g_calls.Th(0x00405750, 0x00785de8u);
        return;
    }
    g_calls.Cd(0x0040cb80);
    if (reinterpret_cast<u8 &>(gCameraFlags) & 0x16) global<u8>(0x00878e84) = 1;
    u32 now;
    g_calls.Cd(0x0067d3f0, &now);
    char line[0x100];
    g_calls.Cd(0x0067a650, line, (u32)(uintptr_t)&gGamestart_764fac, (u32)(uintptr_t)&gGamestart_764fcc, (u32)(uintptr_t)&gGamestart_764fd8, now);
    gGamestart_879068 = 1;
    g_calls.Cd(0x0067b5b0, global<u32(__stdcall *)()>(IMPORT(Kernel32_GetTickCount))());
    g_calls.Cd(0x0051b660, 0x00786de4u, gGraphicssettings_7a1a8c & 0x400);
    g_calls.Cd(0x0051b920, global<u32>(0x0078d8e0) ? global<s32>(0x0078591c) : global<s32>(0x0078591c) / 2);
    g_calls.Cd(0x0051b3d0);
    g_calls.Cd(0x0051bb60, 0x64u);
    g_calls.Cd(0x0051bb80, global<u32>(0x0078592c));
    g_calls.Cd(0x0051bbc0, global<u32>(0x00785930));
    g_calls.Cd(0x0051bba0, global<u32>(0x00785928));
    g_calls.Cd(0x0051eae0);
    g_calls.Cd(0x0051bfc0, 0u, gSoundCategories.ui, 0xd2u, 0u, 0u, 0u);
    g_calls.Cd(0x0051bd70);
    g_calls.Cd(0x0053e920);
    g_calls.Th(0x00587380, kLoadScreen, g_calls.Th(0x0041a820, 0x0078b530u, (u32)(uintptr_t)&gGameframe_764fe4, 0x1f4u));
    g_calls.Th(0x004813c0, 0x007b51f0u);
    g_calls.Th(0x005879c0, kLoadScreen);
    global<u32>(0x00875030) = 0;
    global<u32>(0x0087906c) = gGraphicssettings_7a1a8c & 0x200 ? 4 : 9;
    global<u32>(0x00875034) = g_calls.Th(0x00402d70, kClock);
    g_calls.Cd(0x005989c0);
    gGameframe_878128 = 0;
    gGamestart_878d64 = 1;
}

void __cdecl Game_PlayLogoMovie() {
    String movie;
    String_Ctor(&movie);
    setPath((u32)(uintptr_t)&gGamestart_764ff4);
    if (lookUp(&movie)) {
        g_calls.Cd(0x00583790);
        play(&movie);
    }
    String_Dtor(&movie);
}

void __cdecl Game_PlayDayMovie() {
    String movie;
    String_Ctor(&movie);
    u32 now;
    g_calls.Cd(0x0067d3f0, &now);
    const s32 day = ptr<const s32 *>(g_calls.Cd(0x0067d8e0, &now))[3] % 8;
    if ((u32)day < 8) setPath(kDayMovies[day]);
    if (lookUp(&movie)) play(&movie);
    String_Dtor(&movie);
}
