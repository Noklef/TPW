#include "gameshutdown.h"

#include <cstdint>

#include "string.h"
#include "vcall.h"
#include "../files/memsource.h"
#include "gamestart.h"

namespace {

AddrCalls g_calls;

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

}  // namespace

void GameShutdown_SetCalls(const AddrCall *table, u32 count) { g_calls.Set(table, count); }

extern "C" {

u32 __cdecl Game_ShutdownOwner() { return g_calls.Th(0x00409150, 0x00786b68u); }

u32 __cdecl Game_ShutdownEngine() {
    g_calls.Cd(0x00572b60);
    g_calls.Cd(0x00583700);
    return g_calls.Cd(0x005633a0);
}

void __cdecl Game_RemoveTempFolder() {
    String path;
    String_Ctor(&path);
    vcall<void>(&path, 0x18, MemorySource_GetLock(reinterpret_cast<MemorySource *>((uintptr_t)0x00f816c0)));
    String_EnsureTrailingBackslash(&path);
    vcall<void>(&path, 0x28, reinterpret_cast<const void *>((uintptr_t)0x00878eb8));
    g_calls.Cd(0x005c5250, &path);
    String_Dtor(&path);
}

void __cdecl Game_Shutdown() {
    if (!gGamestart_879068) return;
    if (global<u32>(0x0087906c) == 0xa) {
        global<u32>(0x0087906c) = 0xb;
        g_calls.Cd(0x0054e360);
    }
    Game_ShutdownOwner();
    g_calls.Th(0x004813d0, 0x007b51f0u);
    static const u32 kCalls[] = {0x0040d070u, 0x0051b3e0u, 0x0051bc70u, 0x0051eda0u, 0x0051ee40u, 0x0051b7c0u, 0x0053e930u, 0x0051fd20u, 0x005584b0u, 0x0055d4d0u, 0x00598ad0u};
    for (const u32 call : kCalls) g_calls.Cd(call);
    Game_RemoveTempFolder();
}
}
