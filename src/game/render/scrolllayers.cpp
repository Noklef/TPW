#include "scrolllayers.h"

#include <cstdint>

#include "../math/x87.h"

namespace {

AddrCalls g_calls;

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
u32 &global(u32 address) { return *reinterpret_cast<u32 *>((uintptr_t)address); }

void touch(u32 item) { g_calls.Th(0x0058c9f0, global(0x008c4b44), item, 0u); }

void wrap(float &x) {
    if (x > 1.0f || !(x >= -1.0f)) {
        const s32 whole = (s32)(long long)X87Reg(x).v;
        x = (float)(x - X87Reg((double)whole));
    }
}

}  // namespace

void ScrollLayers_SetCalls(const AddrCall *table, u32 count) { g_calls.Set(table, count); }

extern "C" {

u32 __fastcall ScrollLayers_Update(u8 *self) {
    if (!at<u32>(self, 0x2ec8) || !at<u32>(self, 0x1fd0) || !at<u32>(self, 0x1fd8) || global(0x008bd524) == 4 || (global(0x008bcbc8) & 0x80000000))
        return 1;
    if (global(0x008bd508) & 0xc0000000)
        g_calls.Th(0x00590f90, at<u32>(self, 0x1fd0));
    touch(at<u32>(self, 0x1fd0));
    touch(at<u32>(self, 0x1fd8));
    const X87Reg scaled = X87Reg(at<float>(reinterpret_cast<const void *>((uintptr_t)global(0x008bcbac)), 0x9d98)) * 25.0f;
    float step = (float)scaled;
    if (scaled.v > 25.0 || !(step >= -25.0f) || (global(0x008bd508) & 4))
        step = 0.0f;
    u8 *layer = self + 0x1fa0;
    for (s32 i = 0; i < at<s32>(self, 0x2ec8); ++i, layer += 0xc) {
        at<float>(layer, 0) = (float)(X87Reg(at<float>(layer - 0x30, 0)) * step + at<float>(layer, 0));
        at<float>(layer, 8) = (float)(X87Reg(step) * at<float>(layer - 0x28, 0) + at<float>(layer, 8));
        wrap(at<float>(layer, 0));
        wrap(at<float>(layer, 8));
    }
    u8 *item = self + 0x1fdc;
    for (s32 i = 0; i < at<s32>(self, 0x21dc); ++i, item += 0x20)
        touch(at<u32>(item, 0));
    return 1;
}
}
