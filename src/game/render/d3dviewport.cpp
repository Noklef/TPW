// The Direct3D setup's viewport: see d3dviewport.h.
#include "d3dviewport.h"

#include <cstdint>

#include "../core/vcall.h"
#include "d3dresult.h"

using namespace d3dresult;

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

}  // namespace

extern "C" {

u32 __fastcall D3dSetup_CreateViewport(u8 *self) {
    u8 **const viewport = reinterpret_cast<u8 **>(self + 0xc);
    keep(comcall<u32>(at<u8 *>(self, 4), 0x18, viewport, 0u));
    if (!pushed(3)) return 0;
    keep(comcall<u32>(at<u8 *>(self, 8), 0x14, *viewport));
    if (!pushed(3)) return 0;
    const s32 width = global<s32>(0x008bcbd0), height = global<s32>(0x008bcbd4);
    u32 view[11] = {0x2c, 0, 0, (u32)width, (u32)height, 0, 0, 0, 0, 0, 0x3f800000};
    at<float>(view, 0x1c) = (float)width;
    at<float>(view, 0x20) = (float)height;
    keep(comcall<u32>(*viewport, 0x44, view));
    if (!pushed(3)) return 0;
    keep(comcall<u32>(at<u8 *>(self, 8), 0x30, *viewport));
    return pushed(3) ? 1 : 0;
}
}
