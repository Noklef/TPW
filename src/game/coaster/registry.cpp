#include "registry.h"

#include <cstdint>

namespace {

template <typename T> T &at(const void *p, u32 offset) {
    return *reinterpret_cast<T *>((uintptr_t)p + offset);
}

const u8 *coaster_at(u32 index) { return reinterpret_cast<u8 *const *>(0x00790bd0)[index]; }
const u8 *first_link() { return *reinterpret_cast<u8 *const *>(0x00790fe0); }

}  // namespace

extern "C" {

u32 __cdecl Coaster_Count() { return *reinterpret_cast<const u32 *>(0x00792214); }

u32 __cdecl CoasterLink_Nodes(u32 n, u32 *value) {
    const u8 *link = first_link();
    for (; n != 0; --n) link = at<const u8 *>(link, 0x18);
    if (value) *value = at<u32>(link, 0x28);
    return at<u32>(at<const u8 *>(at<const u8 *>(link, 0x54), 0x90), 4);
}

u32 __cdecl CoasterLink_Find(u32 key) {
    for (const u8 *link = first_link(); link; link = at<const u8 *>(link, 0x18))
        if (at<u32>(link, 0x24) == key) return at<u32>(link, 0x14);
    return 0;
}

void __cdecl Coaster_GetPair(u32 index, u32 *a, u32 *b) {
    const u8 *coaster = coaster_at(index);
    *a = at<u32>(coaster, 0x100);
    *b = at<u32>(coaster, 0x104);
}

void __cdecl Ride_EntryPosition(u32 index, const u32 *slot, float *out) {
    const u8 *ride = reinterpret_cast<u8 *const *>(0x007a4610)[index];
    const u8 *entries = at<const u8 *>(at<const u8 *>(at<const u8 *>(ride, 8), 0x28), 4);
    const u8 *object = at<const u8 *>(entries, *slot * 0x14 + 4);
    u32 *dst = reinterpret_cast<u32 *>(out);
    if (!object) {
        dst[2] = dst[1] = dst[0] = 0;
        return;
    }
    dst[0] = at<u32>(object, 0x30);
    dst[1] = at<u32>(object, 0x34);
    dst[2] = at<u32>(object, 0x38);
}

u8 *__cdecl CoasterLinks_FindTile(u32 x, u32 y, u32 *key) {
    const u8 *map = at<const u8 *>(*reinterpret_cast<const u8 *const *>(0x007a0854), 0x6c);
    const u32 tile = at<u32>(map, 0x18) * y + x;
    u8 *found = nullptr;
    for (const u8 *link = *reinterpret_cast<const u8 *const *>(0x00790fe0); link && !found; link = at<const u8 *>(link, 0x18))
        for (found = at<u8 *>(link, 0x4c); found; found = at<u8 *>(found, 0x28))
            if (!(at<u8>(found, 4) & 5) && at<u32>(found, 8) == tile) break;
    if (key) *key = tile;
    return found;
}
}
