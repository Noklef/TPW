#include "rasteriser.h"

#include <cstdint>
#include <mmintrin.h>

#include "../math/x87.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

}  // namespace

static AddrCalls g_speedCalls;

void Rasteriser_SetSpeedCalls(const AddrCall *table, u32 count) { g_speedCalls.Set(table, count); }

extern "C" {

u32 __stdcall Rasteriser_ChooseSpan(u32 flags) {
    u32 routine;
    if (flags & 0x100) {
        if (flags & 0x200000) {
            routine = global<u32>(0x006f19f8);
        } else {
            global<u32>(0x00fbe010) = (((flags & 0x200) ? 0xe0 : 0) + 0x10) >> 2;
            routine = (flags & 0x800) ? global<u32>(0x006f19f4) : global<u32>(0x006f1a20);
        }
    } else if (flags & 0x10000000) {
        if (flags & 0x20000000)
            routine = global<u32>(0x006f1a1c);
        else if (flags & 0x40000000)
            routine = global<u32>(0x006f1a18);
        else
            routine = global<u32>(0x006f1a14);
    } else {
        routine = global<u32>(0x006f19f0);
    }
    global<u32>(0x00fbe044) = routine;
    return 1;
}

u32 __cdecl Rasteriser_ClearMmx() {
    _mm_empty();
    return 1;
}

u32 __stdcall Setting569f30_Set(u32, u32 value) {
    if (value)
        global<u32>(0x00fbe038) = value;
    else
        global<u32>(0x00fbe044) = global<u32>(0x006f1a10);
    return 1;
}

u32 __stdcall SoftRenderer_PixelAt(s32 x, s32 y, u32 *out) {
    const u32 *buffer = at<const u32 *>(global<u8 *>(0x00879dac), 8);
    *out = buffer[(u32)y * global<u32>(0x008bcbd0) + (u32)x];
    return 1;
}
}

double __cdecl Rasteriser_SpeedTest() {
    const u32 width = global<u32>(0x008bcbd0), height = global<u32>(0x008bcbd4);
    const s32 pixels = (s32)(height * width * 50);
    const u32 right = width << 4, bottom = height << 4;
    global<u32>(0x00fbe044) = global<u32>(0x006f1a10);
    u32 corners[4][3] = {{0, 0, 0}, {right, 0, 0}, {right, bottom, 0}, {0, bottom, 0}};
    const u32 start = g_speedCalls.Cd(0x0057cb90);
    for (u32 i = 0; i < 50; ++i) {
        g_speedCalls.Cd(0x006ef7f0, (u32)(uintptr_t)corners[0], (u32)(uintptr_t)corners[1], (u32)(uintptr_t)corners[2]);
        g_speedCalls.Cd(0x006ef7f0, (u32)(uintptr_t)corners[0], (u32)(uintptr_t)corners[2], (u32)(uintptr_t)corners[3]);
    }
    _mm_empty();
    const s32 ticks = (s32)((g_speedCalls.Cd(0x0057cb90) - start) << 4);
    return (X87Reg((double)pixels) / X87Reg((double)ticks)).v;
}
