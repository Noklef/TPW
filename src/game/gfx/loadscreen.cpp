#include "loadscreen.h"

#include <cstdint>

#include "../core/gameheap.h"
#include "../core/vcall.h"
#include "../math/x87.h"
#include "../render/state.h"
#include "mmxfill.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename T> T &at(u8 *base, u32 offset) { return *reinterpret_cast<T *>(base + offset); }

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
        u32 desc[0x1f] = {};
        desc[0] = 0x7c;
        if (reinterpret_cast<s32(__stdcall *)(void *, void *, u32 *, u32, u32)>((*reinterpret_cast<void ***>(surface))[0x19])(surface, nullptr, desc, 1, 0))
            return 1;
        u8 *const lines = reinterpret_cast<u8 *>((uintptr_t)desc[9]);
        if (*reinterpret_cast<u32 *>(image + 0xc) == 2)
            copyRows(image, *reinterpret_cast<const u16 *const *>(image), lines, desc);
        else
            copyRows(image, *reinterpret_cast<const u32 *const *>(image), lines, desc);
        // (the vtable read again for the Unlock, as the original does)
        reinterpret_cast<s32(__stdcall *)(void *, void *)>((*reinterpret_cast<void ***>(surface))[0x20])(surface, nullptr);
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

u32 __fastcall LoadScreen_DrawBar(u8 *screen) {
    const s32 percent = (s32)(long long)(X87Reg((double)*reinterpret_cast<s32 *>(screen)) * *reinterpret_cast<float *>(screen + 0x18)).v;  // __ftol
    const u32 w = global<u32>(0x008bcbd0), h = global<u32>(0x008bcbd4);
    const s32 scale = (s32)(w << 16) / 640;
    const s32 b = (s32)(h * 11) / 480;
    const s32 left = (s32)(w * 14) / 640;
    const s32 top = (s32)(h * 446) / 480;
    u8 *renderer = global<u8 *>(0x00879ab0);
    vcall<u32>(renderer, 0x1c);
    const s32 bar = percent > 100 ? 0x13e : (s32)((u32)percent * 318) / 100;
    const s32 e = b / 5;
    const s32 right = (s32)((u32)((s32)((u32)(bar + 1) * (u32)scale) >> 16) + (u32)left);
    const float x0 = (float)left, y0 = (float)top, x1 = (float)right;
    renderer = global<u8 *>(0x00879ab0);
    if (!vcall<u32>(renderer, 0x28, x0, y0, x1, (float)(s32)((u32)e + (u32)top), 0xff800000u, 0u))
        return 0;
    const s32 f = (s32)((u32)b * 4) / 5;
    renderer = global<u8 *>(0x00879ab0);
    if (!vcall<u32>(renderer, 0x28, x0, (float)(X87Reg((double)e) + y0), x1, (float)(s32)((u32)f + (u32)top), 0xffff0000u, 0u))
        return 0;
    renderer = global<u8 *>(0x00879ab0);
    if (!vcall<u32>(renderer, 0x28, x0, (float)(X87Reg((double)f) + y0), x1, (float)(s32)((u32)top + (u32)b), 0xff800000u, 0u))
        return 0;
    renderer = global<u8 *>(0x00879ab0);
    vcall<u32>(renderer, 0x20);
    *reinterpret_cast<s32 *>(screen + 0xc) = bar;
    *reinterpret_cast<s32 *>(screen + 4) = bar;
    return 1;
}

u32 __fastcall LoadScreen_Tick(u8 *screen) {
    u32(__stdcall *const timeGetTime)() = *reinterpret_cast<u32(__stdcall *const *)()>((uintptr_t)0x006fd3ec);
    if (at<u32>(screen, 0) == 0) {
        at<u32>(screen, 0x28) = timeGetTime();
        at<u32>(screen, 0x2c) = 0;
        if (global<u32>(0x008bd524) == 4) {
            if (u8 *const buffer = at<u8 *>(global<u8 *>(0x00879dac), 4))
                Mmx_FillRows(global<s32>(0x008bcbd0), global<u32>(0x008bcbd4), buffer, global<s32>(0x008bcbd0) * 2);
        }
    }
    timeGetTime();   // (its answer unused)
    u8 *const first = at<u8 *>(screen, 0x10);
    if (!first && !at<u32>(screen, 0x14)) {
        const s32 steps = at<s32>(screen, 0x2c);
        at<s32>(screen, 0x2c) = steps + 1;
        if (steps < 3)
            RenderDevice_Call0();
    } else {
        u8 *image = nullptr;
        if (global<u32>(0x008c478c) == 1) {
            if (at<u32>(screen, 0x14) && at<s32>(screen, 0x2c) < 3) {
                if (at<s32>(screen, 0x2c) == 0)
                    global<u32>(0x008c4780) = timeGetTime();
                image = at<u8 *>(screen, 0x14);
            }
        } else if (first && at<s32>(screen, 0x2c) < 3) {
            image = first;
        }
        if (image) {
            LoadScreen_ShowImage(image);
            global<u32>(0x008bd508) |= 0x8000000;
            ++at<s32>(screen, 0x2c);
        }
    }
    ++at<u32>(screen, 0);
    if (!at<u32>(screen, 0x14) && !LoadScreen_DrawBar(screen))
        return 0;
    return Display_RestoreIfFocused() ? 1 : 0;
}
}
