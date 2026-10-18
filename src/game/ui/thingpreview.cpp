#include "thingpreview.h"

#include <cstdint>
#include <cstring>

#include "../math/x87.h"
#include "../sprite/screenmatrix.h"
#include "../sprite/script.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

template <typename T> T at(const void *p, u32 offset) { return *reinterpret_cast<const T *>((uintptr_t)p + offset); }

u32 bits(float f) {
    u32 b;
    std::memcpy(&b, &f, 4);
    return b;
}

// __ftol's low half: a NaN or a value past the int64 range gives the indefinite, whose low half is 0.
u32 FtolLow(X87Reg x) { return (u32)(long long)x.v; }
}

extern "C" {

u32 __cdecl ThingPreview_Show(const u8 *rect, u8 *thing, s32 depth, u32 scale) {
    if (global<u32>(0x007c24d8)) SpriteScript_Kill(global<s32>(0x007c24a0));
    global<u8 *>(0x007c24d8) = thing;
    const s32 script = at<s32>(thing, 0xc);
    if (!script) {
        global<u32>(0x007c24d8) = 0;
        return (u32)(uintptr_t)thing;
    }
    const s32 left = at<s16>(rect, 4), top = at<s16>(rect, 6), right = at<s16>(rect, 8), bottom = at<s16>(rect, 0xa);
    const float x = (float)X87_MulIntDouble(((right - left) >> 1) + left - 1000, global<float>(0x006ff154));
    float lift;
    std::memcpy(&lift, &scale, 4);
    const u32 sunk = FtolLow(X87Reg(lift) * X87Reg(global<float>(0x006ff160)));  // * -0.25
    const u32 row = (u32)((bottom - top) >> 2) - sunk - (u32)bottom + 0x300;
    const float y = (float)X87_MulIntDouble((s32)row, global<float>(0x006ff150));  // / 1024
    const float z = (float)Sprite_FlipY(depth);
    const s32 slot = SpriteScript_Clone(script, bits(x), bits(y), bits(z), scale);
    global<s32>(0x007c24a0) = slot;
    return (u32)slot;
}

void __cdecl ThingPreview_Hide() {
    if (!global<u32>(0x007c24d8)) return;
    SpriteScript_Kill(global<s32>(0x007c24a0));
    global<u32>(0x007c24d8) = 0;
}
}
