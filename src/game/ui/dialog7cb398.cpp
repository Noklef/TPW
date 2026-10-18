#include "dialog7cb398.h"

#include <cstdint>

#include "../core/vcall.h"
#include "../world/thingscan.h"
#include "../world/world.h"
#include "window.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

}  // namespace

extern "C" {

void __cdecl Dialog7cb398_ShowTopThree() {
    u8 *const panel = Window_FindChild(global<u8 *>(0x007cb398), nullptr, 0x4a68d1);
    if (!panel) return;
    u8 top[3];
    GameTable7cfb90_Get1da720_519510(global<u8 *>(0x007cf83c));   // (the original's ECX for the call below, which ignores it)
    Things_TopThreeValues(&top[0], &top[1], &top[2]);
    static const u32 kIds[3] = {0x4a68d4, 0x4a68d3, 0x4a68d2};
    for (int k = 0; k < 3; ++k) {
        u8 *const child = Window_FindChild(panel, nullptr, kIds[k]);
        if (top[k] != 0 && top[k] < 0x17) vcall<void>(child, 0x1c, (u32)top[k] - 1);
        else Window_SetShown(child, nullptr, 0);
    }
}

}
