#include "faces.h"

#include <cstdint>

namespace {

template <typename T> T &at(const void *p, u32 offset) {
    return *reinterpret_cast<T *>((uintptr_t)p + offset);
}

}  // namespace

extern "C" {

void __cdecl Face_MarkTextures(u32, const u8 *face, u32 set) {
    if (at<u32>(face, 0) & 0x200) return;
    const u8 *entry = at<const u8 *>(face, 0x6c);
    for (u32 n = at<u16>(face, 0x5a); n; --n, entry += 0x10) {
        u8 *const texture = at<u8 *>(entry, 0);
        if (set) texture[1] |= 8;
        else at<u16>(texture, 0) &= 0xf7ff;
    }
}
}
