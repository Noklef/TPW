#include "wctimage.h"

#include <cstdint>
#include "../core/gameheap.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

extern "C" {

u8 *__fastcall WctImage_DeletingDtor(u8 *part, void *, u32 flags) {
    u8 *image = part - 0x2c;
    at<u32>(image, at<u32>(at<u8 *>(image, 0), 4)) = 0x007011c8;
    const u32 base = at<u32>(at<u8 *>(image, 0), 4);
    at<u32>(image, base - 4) = base - 0x2c;
    if (at<u32>(image, 0x20)) {
        TrackedGlobal_Free(at<u32>(image, 0x20));
        at<u32>(image, 0x20) = 0;
    }
    if (at<u32>(image, 0x24)) {
        TrackedGlobal_Free(at<u32>(image, 0x24));
        at<u32>(image, 0x24) = 0;
    }
    for (u32 offset = 4; offset < 0x20; offset += 4) at<u32>(image, offset) = 0;
    at<u32>(image, 0x2c) = 0x00701178;
    if (flags & 1) GameHeap_Free(image);
    return image;
}
}
