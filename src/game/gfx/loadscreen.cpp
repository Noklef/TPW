#include "loadscreen.h"

#include <cstdint>

#include "../core/gameheap.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

void close(u8 *screen) {
    if (global<u32>(0x008c478c)) {
        u32(__stdcall *const timeGetTime)() = *reinterpret_cast<u32(__stdcall *const *)()>((uintptr_t)0x006fd3ec);
        if (timeGetTime() - global<u32>(0x008c4780) < 3000) {
            while (timeGetTime() - global<u32>(0x008c4780) < 3000) {
            }
        }
    }
    for (u32 offset = 0x10; offset <= 0x14; offset += 4) {
        u32 *const block = *reinterpret_cast<u32 **>(screen + offset);
        if (!block)
            continue;
        if (*block) {
            TrackedGlobal_Free(*block);
            *block = 0;
        }
        GameHeap_Free(block);
        *reinterpret_cast<u32 **>(screen + offset) = nullptr;
    }
    global<u32>(0x008bcbc8) &= ~0x40000000u;
}

// The scaled, upside-down copy of the image's pixels (T: u16 or u32) into a locked surface's rows (pitch in bytes).
template <typename T> void copyRows(const u8 *image, const T *pixels, u8 *row, const u32 *desc) {
    for (s32 y = 0; y < static_cast<s32>(desc[2]); ++y) {
        T *out = reinterpret_cast<T *>(row);
        for (s32 x = 0; x < static_cast<s32>(desc[3]); ++x) {
            const u32 width = *reinterpret_cast<const u32 *>(image + 4);
            const u32 from = (desc[2] - y - 1) * *reinterpret_cast<const u32 *>(image + 8) / desc[2];
            *out++ = pixels[from * width + x * width / desc[3]];
        }
        row += desc[4];
    }
}

}  // namespace

extern "C" {

u32 __fastcall LoadScreen_ShowImage(u8 *image) {
    const u8 *const pixels = *reinterpret_cast<u8 *const *>(image);
    if (!pixels)
        return 1;
    if (global<u32>(0x008bd524) == 1) {
        void *const surface = *reinterpret_cast<void **>(global<u8 *>(0x00879a24) + 0x14);
        void **const slots = *reinterpret_cast<void ***>(surface);
        u32 desc[0x1f] = {};
        desc[0] = 0x7c;
        if (reinterpret_cast<s32(__stdcall *)(void *, void *, u32 *, u32, u32)>(slots[0x19])(surface, nullptr, desc, 1, 0))
            return 1;
        u8 *const lines = reinterpret_cast<u8 *>((uintptr_t)desc[9]);
        if (*reinterpret_cast<u32 *>(image + 0xc) == 2)
            copyRows(image, *reinterpret_cast<const u16 *const *>(image), lines, desc);
        else
            copyRows(image, *reinterpret_cast<const u32 *const *>(image), lines, desc);
        reinterpret_cast<s32(__stdcall *)(void *, void *)>(slots[0x20])(surface, nullptr);
        return 1;
    }
    if (global<u32>(0x008bd524) == 4) {
        u32 *out = *reinterpret_cast<u32 **>(global<u8 *>(0x00879dac) + 8);
        for (s32 y = 0; y < global<s32>(0x008bcbd4); ++y) {
            for (s32 x = 0; x < global<s32>(0x008bcbd0); ++x) {
                const s32 width = *reinterpret_cast<const s32 *>(image + 4);
                const s32 rows = global<s32>(0x008bcbd4);
                const s32 from = (rows - y - 1) * *reinterpret_cast<const s32 *>(image + 8) / rows;
                *out++ = reinterpret_cast<const u32 *>(pixels)[from * width + x * width / global<s32>(0x008bcbd0)];
            }
        }
    }
    return 1;
}

u32 __fastcall LoadScreen_Close(u8 *screen) {
    close(screen);
    return 1;
}

void __cdecl LoadScreen_StaticDtor() { close(reinterpret_cast<u8 *>((uintptr_t)0x008c4748)); }
}
