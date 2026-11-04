// A class with a list of windows: see windowgroup.h.
#include "windowgroup.h"

#include <cstdint>

#include "../core/vcall.h"

namespace {
AddrCalls g_calls;
}

void WindowGroup_SetCalls(const AddrCall *table, u32 count) { g_calls.Set(table, count); }

extern "C" {

void __fastcall WindowGroup_TellObject(u8 *self) {
    void *object = *reinterpret_cast<void **>(self + 0x9c);
    if (object) vcall<u32>(object, 8, 0u, self + 0x2c, (u32)(u16)(*reinterpret_cast<u16 *>(self + 0xe0) + 1));
}

void __fastcall WindowGroup_OpenAll(u8 *self) {
    const u32 *const &windows = *reinterpret_cast<const u32 *const *>(self + 0x148);
    if (!windows) return;
    for (s32 i = 0; i < *reinterpret_cast<s16 *>(self + 0x172); ++i) g_calls.Th(0x0065c707, windows[i]);
}
}
