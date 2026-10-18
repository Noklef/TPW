#include "tvstatic.h"

#include <cstdint>

#include "../core/gameheap.h"
#include "../core/vcall.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

}  // namespace

extern "C" {

Surface *__cdecl TvStatic_CreateSurface() {
    Surface *s = static_cast<Surface *>(GameHeap_Alloc(0x50));
    if (s) {
        u8 *key = reinterpret_cast<u8 *>(&s->keyRgb);
        for (u32 i = 0; i < 5; ++i)
            key[i] = 0;
        s->attached = 0;
        s->surface = nullptr;
        SurfaceInfo_Ctor(&s->info);
    }
    u32 desc[0x1b];  // (only the fields below are set, as in the original)
    desc[0] = 0x6c;
    desc[1] = 7;
    desc[2] = 0xf0;
    desc[3] = 0x140;
    desc[0x1a] = 0x840;
    void *surface = s;  // (the original's out slot held the Surface)
    u8 *directDraw = *global<u8 **>(0x00879a24);
    s32 result;
    if (comcall<s32>(directDraw, 0x18, desc, &surface, nullptr) != 0)
        result = -1;
    else
        Surface_Attach(&result, surface, s);
    if (result < 0) {
        GameHeap_Free(s);
        return nullptr;
    }
    return s;
}

void __cdecl TvStatic_Init() { global<Surface *>(0x00874fd0) = TvStatic_CreateSurface(); }

void __cdecl TvStatic_Free() {
    void *surface = Surface_GetSurface(global<Surface *>(0x00874fd0), 1);
    comcall<u32>(surface, 8);
    GameHeap_Free(global<Surface *>(0x00874fd0));
}
}
