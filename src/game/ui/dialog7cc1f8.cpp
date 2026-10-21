#include "dialog7cc1f8.h"

#include <cstdint>

#include "panelparts.h"

using namespace panelparts;

extern "C" {

void __cdecl Dialog7cc1f8_DressStats() {
    u8 *const stats = Window_FindChild(*reinterpret_cast<u8 **>((uintptr_t)0x007cc1f8), nullptr, 0x1ed);
    if (!stats) return;
    text(stats, 0x1f4, 0x7d);
    text(stats, 0x1f3, 0x7e);
    text(stats, 0x1f1, 0x7f);
    text(stats, 0x1f6, 0x80);
    text(stats, 0x1f7, 0x81);
    text(stats, 0x1f5, 0x88);
    text(stats, 0x1f2, 0x88);   // (sic: the same text as 0x1f5's)
    bar(stats, kRideStatBar, 0x1ee, 0x1f4);
    bar(stats, kRideStatBar, 0x1ef, 0xc8);
    bar(stats, kRideStatBar, 0x1f0, 0x64);
}

namespace {

// The stats window's child id shown or hidden (nothing when it has none).
void Show(u8 *stats, u32 id, u32 shown) {
    if (u8 *const child = Window_FindChild(stats, nullptr, id)) Window_SetShown(child, nullptr, shown);
}

}  // namespace

void __cdecl Dialog7cc1f8_ShowStats(u32 mode) {
    u8 *const stats = Window_FindChild(*reinterpret_cast<u8 **>((uintptr_t)0x007cc1f8), nullptr, 0x1ed);
    if (!stats) return;
    static const u32 kIds[8] = {0x1f3, 0x1f1, 0x1f6, 0x1f7, 0x1ee, 0x1f2, 0x1ef, 0x1f0};
    switch (mode) {
    case 0:
    case 1:
    case 3:
        for (const u32 id : kIds) Show(stats, id, mode == 0);
        break;
    case 2:
        Show(stats, 0x1f3, 1);
        Show(stats, 0x1f1, 0);
        Show(stats, 0x1f6, 0);
        Show(stats, 0x1f6, 0);   // (sic: twice)
        Show(stats, 0x1f7, 0);
        Show(stats, 0x1ee, 1);
        Show(stats, 0x1f2, 0);
        Show(stats, 0x1ef, 0);
        Show(stats, 0x1f0, 0);
        break;
    default:
        break;
    }
}
}
