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
}
