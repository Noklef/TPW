#include "statsgraph.h"

#include <cstdint>

#include "../core/circbuf.h"
#include "../world/world.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

constexpr u32 kLines[] = {0x1f100, 0x1f350, 0x1fc90, 0x1fee0, 0x20130, 0x20380, 0x1f5a0, 0x1f7f0};

using Scan = void(__cdecl *)(const u8 *, s32 *, u32);

s32 shown(Scan scan) {
    s32 result = 0;
    const u8 *const record = reinterpret_cast<const u8 *>((uintptr_t)GameTable7cfb90_Get1da720_519510(global<u8 *>(0x007cf83c)));
    for (u32 k = 0; k < 8; ++k)
        if (global<u8>(0x00751fa0) & (1u << k)) scan(record + kLines[k], &result, global<u32>(0x007ca2ec));
    return result;
}

}  // namespace

extern "C" {

s32 __cdecl StatsGraph_ShownMax() { return shown(&CircBufU32_144_MaxInWindow); }

s32 __cdecl StatsGraph_ShownMin() { return shown(&CircBufU32_144_MinInWindow); }

s32 __cdecl StatsGraph7cb318_ShownMax() {
    constexpr u32 kOffsets[] = {0x20cc0, 0x20f10, 0x21160, 0x205d0, 0x213b0};
    s32 result = 0;
    const u8 *const record = reinterpret_cast<const u8 *>((uintptr_t)GameTable7cfb90_Get1da720_519510(global<u8 *>(0x007cf83c)));
    for (u32 k = 0; k < 5; ++k)
        if (global<u8>(0x00753c48) & (1u << k)) (k == 1 ? &CircBufF32_144_MaxInWindow : &CircBufU32_144_MaxInWindow)(record + kOffsets[k], &result, global<u32>(0x007cb39c));
    return result;
}
}
