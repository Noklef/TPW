#include "animtex.h"

#include <cstdint>

#include "../core/vcall.h"
#include "../math/x87.h"
#include "../render/screen.h"
#include "../world/camera.h"

namespace {

u32 *frames() { return reinterpret_cast<u32 *>((uintptr_t)0x008c4b98); }
u32 *counts() { return reinterpret_cast<u32 *>((uintptr_t)0x008c4b80); }

u32 screenWidthNow() { return gScreen_7a1968; }
u32 screenHeightNow() { return gScreen_7a1930; }

// Under the mode 4 (0x8bd524): the frame wholly inside the rectangle of 0x8bcbd0, 0x8bcbd4; its corners tried from the top left.
bool frame_inside(float x, float y, float wide, float tall) {
    const float limitX = (float)*reinterpret_cast<const s32 *>((uintptr_t)0x008bcbd0);
    const float limitY = (float)*reinterpret_cast<const s32 *>((uintptr_t)0x008bcbd4);
    if (!(x >= 0.0f) || !(x < limitX) || !(y >= 0.0f) || !(y < limitY)) return false;
    return (X87Reg(wide) + x).v < (double)limitX && (X87Reg(tall) + y).v < (double)limitY;
}

}  // namespace

extern "C" {

// Each animation's frames, then its count.
void __cdecl AnimTextures_Set(const u32 *from, const u32 *fromCounts) {
    for (int animation = 0; animation < 6; ++animation) {
        for (int i = 0; i < 0x100; ++i)
            frames()[animation * 0x100 + i] = from[animation * 0x100 + i];
        counts()[animation] = fromCounts[animation];
    }
}

void __cdecl AnimTextures_Clear() {
    for (int animation = 0; animation < 6; ++animation)
        counts()[animation] = 0;
}

void __cdecl AnimTextures_DrawSheet() {
    if (!*reinterpret_cast<const u8 *>((uintptr_t)0x008c639c)) return;
    float y = (float)(X87Reg(gCamera_790acc) * -4.0f);
    u32 scroll = 0;
    float wide = 0.0f, tall = 0.0f;   // sic: the original's tall is a stack slot not yet written until an animation of over one frame sets it
    do {
        for (int animation = 0; animation < 6; ++animation) {
            const s32 count = (s32)counts()[animation];
            if (count > 1) {
                u32 *const row = frames() + animation * 0x100;
                const u8 *const first = *reinterpret_cast<const u8 *const *>((uintptr_t)row[0]);
                wide = (float)(X87Reg((double)*reinterpret_cast<const s32 *>(first + 8)) * 1.0f);
                tall = (float)(X87Reg((double)*reinterpret_cast<const s32 *>(first + 0xc)) * 1.0f);
                const s32 offset = (s32)(scroll + *reinterpret_cast<const u32 *>((uintptr_t)0x008c63a0) * 0x102);
                const s32 step = (s32)(long long)wide + 2;   // (_ftol)
                s32 i = offset / step - 1;
                if (i < 0) i = 0;
                float x = (float)(X87Reg((double)(offset % step)) - X87Reg((double)step));
                if (i < count) {
                    const float advance = (float)(X87Reg(wide) - -2.0f);
                    do {
                        const u32 texture = *reinterpret_cast<const u32 *>((uintptr_t)row[i]);
                        const s32 mode = *reinterpret_cast<const s32 *>((uintptr_t)0x008bd524);
                        if (mode != 4 || frame_inside(x, y, wide, tall)) {
                            u8 *const renderer = *reinterpret_cast<u8 **>((uintptr_t)0x00879ab0);
                            vcall<u32>(renderer, 0x28, x, y, (float)(X87Reg(wide) + x), (float)(X87Reg(tall) + y), 0xffffffffu, texture);
                        }
                        const X87Reg right = X87Reg(x) + advance;
                        x = (float)right;
                        const u32 screenWidth = screenWidthNow();
                        if (right.v > (double)screenWidth) break;
                        ++i;
                    } while (i < (s32)counts()[animation]);
                }
            }
            const X87Reg below = (X87Reg(tall) - -2.0f) + y;
            y = (float)below;
            const u32 screenHeight = screenHeightNow();
            if (below.v > (double)screenHeight) break;
        }
        scroll += screenWidthNow();
    } while ((double)screenHeightNow() > (double)y);
}
}
