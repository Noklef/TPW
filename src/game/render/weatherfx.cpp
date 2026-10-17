#include "weatherfx.h"

#include <cstdint>

#include "../core/gameheap.h"
#include "../gfx/texture.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

extern "C" {

void __fastcall WeatherFx_SetDropCount(u8 *fx, void *, u32 count) { at<u32>(fx, 0x3318) = count; }

void __fastcall WeatherFx_SetFallSpeed(u8 *fx, void *, u32 speed) {
    u32 kinds = at<u32>(fx, 0);
    if (kinds & 1)
        at<u32>(fx, 0x3338) = speed;
    if (kinds & 2)
        at<u32>(fx, 0x333c) = speed;
}

void __fastcall WeatherFx_RequestReload(u8 *fx) { at<u32>(fx, 4) = 1; }

u32 __cdecl WeatherFx_Delete() {
    u8 *&slot = *reinterpret_cast<u8 **>((uintptr_t)0x008bcbac);
    if (!slot)
        return 0;
    WeatherFx_FreeTextures(slot);
    GameHeap_Free(slot);
    slot = nullptr;
    return 1;
}

u32 __fastcall WeatherFx_FreeTextures(u8 *fx) {
    for (u32 offset = 0x3348; offset <= 0x3350; offset += 4) {
        if (!at<u8 *>(fx, offset))
            continue;
        Texture_Release(at<u8 *>(fx, offset));
        if (u8 *const array = at<u8 *>(fx, offset))
            Texture_DestroyArray(array);
        at<u32>(fx, offset) = 0;
    }
    return 1;
}
}
