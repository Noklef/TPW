#include "land.h"

#include <cstdint>

namespace {

template <typename T> T &at(const void *p, u32 offset) {
    return *reinterpret_cast<T *>((uintptr_t)p + offset);
}

const u8 *map() { return *reinterpret_cast<const u8 *const *>(0x007a0854); }

}  // namespace

extern "C" {

u32 __cdecl Terrain_IsParkLand(u32 x, u32 y) {
    const u8 *grid = map() ? at<const u8 *>(map(), 0x6c) : nullptr;
    if (!grid) return 1;
    const u32 width = at<u32>(grid, 0x18);
    if (x >= width || y >= at<u32>(grid, 0x1c)) return 0;
    return ~at<const u8 *>(grid, 0x2c)[(y * width + x) * 4] & 1;
}

void __cdecl Terrain_CopyAttrs(const u8 *map, const u8 *grid) {
    if (!map) return;
    const u8 *own = at<const u8 *>(map, 0x6c);
    if (!own || !at<u8 *>(own, 0x2c) || !at<const u8 *>(grid, 0x2c) || !at<u16>(own, 6)) return;
    u32 *to = at<u32 *>(own, 0x2c);  // rep movsd: forward, a dword at a time
    const u32 *from = at<const u32 *>(grid, 0x2c);
    for (u32 n = at<u16>(own, 6); n; --n) *to++ = *from++;
}

void __cdecl Terrain_GridSize(u32 *width, u32 *height) {
    *width = at<u32>(at<const u8 *>(map(), 0x6c), 0x18);
    *height = at<u32>(at<const u8 *>(map(), 0x6c), 0x1c);
}
}
