#include "softrender.h"

#include <cstdint>
#include <cstring>

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

extern "C" {

u32 __fastcall SoftRenderer_ClearBuffer(u8 *renderer) {
    std::memset(at<u8 *>(renderer, 8), 0, at<u32>(renderer, 0xc));
    return 1;
}

u32 __fastcall SoftRenderer_TextureMemory(u8 *) {
    u32 option = *reinterpret_cast<u32 *>(0x008c1620);
    if (option) return option;
    s32 quarter = *reinterpret_cast<s32 *>(0x008bd520) / 4;
    if (quarter > 0x1800000) return 0x1800000;
    if (quarter < 0x800000) return 0x800000;
    return (u32)quarter;
}
}
