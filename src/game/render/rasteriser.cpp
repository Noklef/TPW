#include "rasteriser.h"

#include <cstdint>
#include <mmintrin.h>

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

}  // namespace

extern "C" {

u32 __stdcall Rasteriser_ChooseSpan(u32 flags) {
    u32 routine;
    if (flags & 0x100) {
        if (flags & 0x200000) {
            routine = global<u32>(0x006f19f8);
        } else {
            global<u32>(0x00fbe010) = (((flags & 0x200) ? 0xe0 : 0) + 0x10) >> 2;
            routine = (flags & 0x800) ? global<u32>(0x006f19f4) : global<u32>(0x006f1a20);
        }
    } else if (flags & 0x10000000) {
        if (flags & 0x20000000)
            routine = global<u32>(0x006f1a1c);
        else if (flags & 0x40000000)
            routine = global<u32>(0x006f1a18);
        else
            routine = global<u32>(0x006f1a14);
    } else {
        routine = global<u32>(0x006f19f0);
    }
    global<u32>(0x00fbe044) = routine;
    return 1;
}

u32 __cdecl Rasteriser_ClearMmx() {
    _mm_empty();
    return 1;
}

u32 __stdcall Setting569f30_Set(u32, u32 value) {
    if (value)
        global<u32>(0x00fbe038) = value;
    else
        global<u32>(0x00fbe044) = global<u32>(0x006f1a10);
    return 1;
}

u32 __stdcall SoftRenderer_PixelAt(s32 x, s32 y, u32 *out) {
    const u32 *buffer = at<const u32 *>(global<u8 *>(0x00879dac), 8);
    *out = buffer[(u32)y * global<u32>(0x008bcbd0) + (u32)x];
    return 1;
}
}
