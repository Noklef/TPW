#include "screensurfaces.h"

#include <cstdint>

#include "surface.h"
#include "../core/vcall.h"
#include "../core/gameheap.h"
#include "../render/screen.h"
#include "../render/d3dsetup.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>(address); }

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
    at<u32>(source, 0) = (u32)(uintptr_t)SurfaceSource_vtable;
    initSurface(source + 4);
    initSurface(source + 0x54);
    at<u32>(source, 0) = (u32)(uintptr_t)Class6fef78_vtable;
    at<u32>(source, 0xa8) = 0;
    at<u32>(source, 0xa4) = 0;
    at<u32>(source, 0xac) = 0;
    at<u32>(source, 0xb0) = 0;
    return source;
}

void __fastcall SurfaceSource_Dtor(u8 *source) { at<u32>(source, 0) = (u32)(uintptr_t)SurfaceSource_vtable; }

}

namespace {

// The display's surface at `member` queried for the interface 0x707210 into `iface`, then wrapped by the Surface at `surface`;
// once (done).
u8 *wrapOnce(u8 *source, u32 done, u32 member, u32 iface, u32 surface) {
    if (!at<u32>(source, done)) {
        void *const display = at<void *>(global<void *>((u32)(uintptr_t)&gD3dsetup_879a20 + 0x4), member);
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
    const s32 right = (s32)(gScreen_7a1968 - 1);
    const s32 bottom = (s32)(gScreen_7a1930 - 1);
    out[0] = 0;
    out[1] = 0;
    out[2] = right;
    out[3] = bottom;
    return out;
}
void __fastcall ScreenSurfaces_Dtor(u8 *object) {
    at<u32>(object, 0) = (u32)(uintptr_t)Class6fef78_vtable;
    u8 *second = at<u8 *>(object, 0xb0);
    at<u32>(object, 0xa8) = 0;
    at<u32>(object, 0xa4) = 0;
    if (second)
        comcall<u32>(second, 8);
    if (u8 *first = at<u8 *>(object, 0xac))
        comcall<u32>(first, 8);
    at<u32>(object, 0xac) = 0;
    at<u32>(object, 0xb0) = 0;
    at<u32>(object, 0) = (u32)(uintptr_t)SurfaceSource_vtable;
}

u8 *__fastcall ScreenSurfaces_ScalarDtor(u8 *object, void *, u32 flags) {
    ScreenSurfaces_Dtor(object);
    if (flags & 1)
        GameHeap_Free(object);
    return object;
}
}
