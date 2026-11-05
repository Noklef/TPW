#include "devicereset.h"

#include <cstdint>

#include "../core/vcall.h"

namespace {

AddrCalls g_calls;

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

bool DisplayRestored() {
    return g_calls.Th(0x00564140, global<u32>(0x879a24)) && g_calls.Th(0x00564160, global<u32>(0x879a24));   // then Display_RestoreOrOpen
}

bool Restored() {
    if (global<u32>(0x8bd524) != 1) return DisplayRestored();
    return g_calls.Th(0x0058dc10, global<u32>(0x8c4b44))   // TexCache_ReleaseGraphics
        && vcall<u32>(global<u8 *>(0x8bd52c), 0x14)
        && DisplayRestored()
        && vcall<u32>(global<u8 *>(0x8bd52c), 0x18)
        && g_calls.Th(0x0058db90, global<u32>(0x8c4b44), 0xffffffffu)   // TexCache_ChangeFormat
        && vcall<u32>(global<u8 *>(0x879ab0), 4);
}

}  // namespace

void DeviceReset_SetCalls(const AddrCall *table, u32 count) { g_calls.Set(table, count); }

extern "C" {

u32 __cdecl Engine_ResetDevice() {
    if (Restored()) {
        g_calls.Cd(0x0047ea00);
        return 1;
    }
    g_calls.Th(0x0058dc10, global<u32>(0x8c4b44));
    g_calls.Cd(0x00583700);
    g_calls.Th(0x005640a0, global<u32>(0x879a24));
    g_calls.Th(0x0056af20, 0x00879ad0u, 3u, 0xb0754014u);   // EngineErrors_Push
    return 0;
}

}
