#include "screensurfaces.h"

#include <cstdint>

#include "surface.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>(address); }

constexpr u32 kBaseVtable = 0x006fef90;
constexpr u32 kVtable = 0x006fef78;

// A Surface's constructor, inlined: the colour key bytewise, attached and surface, the info.
void initSurface(u8 *s) {
    for (u32 i = 0x10; i < 0x15; ++i) s[i] = 0;
    at<u32>(s, 0x20) = 0;
    at<u32>(s, 0x24) = 0;
    SurfaceInfo_Ctor(reinterpret_cast<SurfaceInfo *>(s + 0x28));
}

}  // namespace

extern "C" {

u8 *__fastcall ScreenSurfaces_Ctor(u8 *source) {
    at<u32>(source, 0) = kBaseVtable;
    initSurface(source + 4);
    initSurface(source + 0x54);
    at<u32>(source, 0) = kVtable;
    at<u32>(source, 0xa8) = 0;
    at<u32>(source, 0xa4) = 0;
    at<u32>(source, 0xac) = 0;
    at<u32>(source, 0xb0) = 0;
    return source;
}

void __fastcall SurfaceSource_Dtor(u8 *source) { at<u32>(source, 0) = kBaseVtable; }

s32 *__fastcall ScreenSurfaces_Bounds(const u8 *, void *, s32 *out) {
    const s32 right = (s32)(global<u32>(0x007a1968) - 1);
    const s32 bottom = (s32)(global<u32>(0x007a1930) - 1);
    out[0] = 0;
    out[1] = 0;
    out[2] = right;
    out[3] = bottom;
    return out;
}
}
