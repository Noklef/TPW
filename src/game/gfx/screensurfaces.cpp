#include "screensurfaces.h"

#include <cstdint>

#include "surface.h"
#include "../core/vcall.h"
#include "../core/gameheap.h"

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

}

namespace {

// The display's surface at `member` queried for the interface 0x707210 into `iface`, then wrapped by the Surface at `surface`;
// once (done).
u8 *wrapOnce(u8 *source, u32 done, u32 member, u32 iface, u32 surface) {
    if (!at<u32>(source, done)) {
        void *const display = at<void *>(global<void *>(0x00879a24), member);
        comcall<s32>(display, 0, reinterpret_cast<const void *>(0x00707210), &at<void *>(source, iface));
        if (void *const s = at<void *>(source, iface)) {
            s32 result;
            Surface_Attach(&result, s, reinterpret_cast<Surface *>(source + surface));
        }
        at<u32>(source, done) = 1;
    }
    return source + surface;
}

}  // namespace

extern "C" {

u8 *__fastcall ScreenSurfaces_Surface10(u8 *source) { return wrapOnce(source, 0xa8, 0x10, 0xac, 0x54); }

u8 *__fastcall ScreenSurfaces_Surface14(u8 *source) { return wrapOnce(source, 0xa4, 0x14, 0xb0, 0x4); }

s32 *__fastcall ScreenSurfaces_Bounds(const u8 *, void *, s32 *out) {
    const s32 right = (s32)(global<u32>(0x007a1968) - 1);
    const s32 bottom = (s32)(global<u32>(0x007a1930) - 1);
    out[0] = 0;
    out[1] = 0;
    out[2] = right;
    out[3] = bottom;
    return out;
}
void __fastcall ScreenSurfaces_Dtor(u8 *object) {
    at<u32>(object, 0) = 0x006fef78;
    u8 *second = at<u8 *>(object, 0xb0);
    at<u32>(object, 0xa8) = 0;
    at<u32>(object, 0xa4) = 0;
    if (second)
        comcall<u32>(second, 8);
    if (u8 *first = at<u8 *>(object, 0xac))
        comcall<u32>(first, 8);
    at<u32>(object, 0xac) = 0;
    at<u32>(object, 0xb0) = 0;
    at<u32>(object, 0) = 0x006fef90;
}

u8 *__fastcall ScreenSurfaces_ScalarDtor(u8 *object, void *, u32 flags) {
    ScreenSurfaces_Dtor(object);
    if (flags & 1)
        GameHeap_Free(object);
    return object;
}
}
