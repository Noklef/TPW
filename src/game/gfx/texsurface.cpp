#include "texsurface.h"

#include <cstdint>

#include "texcache.h"
#include "../core/gameheap.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &back(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p - offset); }

// The virtual base's offset, from the vtable pointer at the object's start.
u32 BaseOffset(const u8 *surface) { return at<u32>(at<const u8 *>(surface, 0), 4); }

}  // namespace

extern "C" {

u8 *__fastcall TexSurfaceA_Ctor(u8 *surface, void *, u32 mostDerived) {
    if (mostDerived) {
        at<u32>(surface, 0) = 0x00702218;
        TexCacheItem_Ctor(surface + 0x3c);
    }
    at<u32>(surface, BaseOffset(surface)) = 0x007021f0;
    const u32 offset = BaseOffset(surface);
    at<u32>(surface, offset - 4) = offset - 0x3c;
    for (u32 i = 0; i < 4; i++) {
        at<u32>(surface, 4 + 4 * i) = 0;
        at<u32>(surface, 0x14 + 4 * i) = 0;
    }
    at<u32>(surface, 0x24) = 0;
    at<u32>(surface, BaseOffset(surface) + 8) = 0;
    at<u32>(surface, BaseOffset(surface) + 0xc) = 0;
    for (u32 i = 0; i < 4; i++)
        at<u32>(surface, 0x28 + 4 * i) = 0;
    return surface;
}

u8 *__fastcall TexSurfaceB_Ctor(u8 *surface, void *, u32 mostDerived) {
    if (mostDerived) {
        at<u32>(surface, 0) = 0x00702278;
        TexCacheItem_Ctor(surface + 0x10);
    }
    at<u32>(surface, BaseOffset(surface)) = 0x00702250;
    const u32 offset = BaseOffset(surface);
    at<u32>(surface, offset - 4) = offset - 0x10;
    at<u32>(surface, BaseOffset(surface) + 0xc) = 0;
    at<u32>(surface, BaseOffset(surface) + 8) = 0;
    at<u32>(surface, 8) = 0;
    at<u32>(surface, 4) = 0;
    return surface;
}

u32 __fastcall TexSurfaceA_Field38(const u8 *sub) { return back<u32>(sub, 0x38); }

u32 __fastcall TexSurfaceA_Area(const u8 *sub) {
    const u32 off = at<u32>(back<const u8 *>(sub, 0x3c), 4);
    return back<u32>(sub + off, 0x30) * back<u32>(sub + off, 0x34);
}

u32 __fastcall TexSurfaceA_Field10(const u8 *sub) { return back<u32>(sub, 0x10); }

u32 __fastcall TexSurfaceB_FieldC(const u8 *sub) { return back<u32>(sub, 0xc); }

u32 __fastcall TexSurfaceB_Area(const u8 *sub) {
    const u32 off = at<u32>(back<const u8 *>(sub, 0x10), 4);
    return back<u32>(sub + off, 8) * back<u32>(sub + off, 4);
}

u8 *__fastcall Renderer_NewSurfaceA(void *) {
    u8 *surface = static_cast<u8 *>(GameHeap_Alloc(0x50));
    surface = surface ? TexSurfaceA_Ctor(surface, nullptr, 1) : nullptr;
    if (!surface) return nullptr;
    return surface + BaseOffset(surface);
}

u8 *__fastcall SoftRenderer_NewSurfaceB(void *) {
    u8 *surface = static_cast<u8 *>(GameHeap_Alloc(0x24));
    surface = surface ? TexSurfaceB_Ctor(surface, nullptr, 1) : nullptr;
    if (!surface) return nullptr;
    return surface + BaseOffset(surface);
}
}
