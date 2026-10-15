#include "parts.h"

#include <cstdint>

namespace {

template <typename T> T &at(const void *p, u32 offset) {
    return *reinterpret_cast<T *>((uintptr_t)p + offset);
}

const u8 *model_of(const u8 *holder) { return at<const u8 *>(at<const u8 *>(holder, 8), 4); }

u8 *face(const u8 *model, u32 part) {
    const u32 index = at<u16>(model, 0x46) + part, first = at<u16>(model, 0x44);
    return index < first ? at<u8 *>(model, 0x70) + index * 0xa0 : at<u8 *>(model, 0x74) + (index - first) * 0x58;
}

template <typename Match> void mark_faces(const u8 *const *holder, Match match) {
    for (u32 i = 0; i < at<u16>(at<const u8 *>(*holder, 4), 0x48); ++i) {
        const u8 *model = at<const u8 *>(*holder, 4);
        if (match(at<u32>(at<const u8 *>(model, 0x7c), i * 0x14 + 8))) at<u32>(face(model, i), 0) |= 0x80000010;
    }
}

u32 find_part(const u8 *holder, u32 mask, u32 key, u32 keyOffset) {
    const u8 *model = model_of(holder);
    const u32 n = at<u16>(model, 0x48);
    const u8 *part = at<const u8 *>(model, 0x7c);
    for (u32 i = 0; i < n; ++i, part += 0x14)
        if (at<u32>(part, keyOffset) == key && (at<u32>(part, 0) & mask)) return i;
    return 0xffffffff;
}

}  // namespace

extern "C" {

void __cdecl Model_MarkFacesByMask(const u8 *const *holder, u32 mask) {
    mark_faces(holder, [mask](u32 v) { return (v & mask) != 0; });
}

void __cdecl Model_MarkFacesNotValue(const u8 *const *holder, u32 value) {
    mark_faces(holder, [value](u32 v) { return v != value; });
}

u32 __cdecl Model_FindPartByKey4(const u8 *holder, u32 mask, u32 key) {
    return find_part(holder, mask & 0x3da1f83 ? mask : 0x3da1f82, key, 4);
}

u32 __cdecl Model_FindPartByKey8(const u8 *holder, u32 mask, u32 key) {
    return find_part(holder, mask & 0x3da1f82 ? mask : 1, key, 8);
}

u32 __cdecl Model_NthPart(const u8 *holder, u32 mask, u32 nth) {
    const u8 *model = model_of(holder);
    const u32 n = at<u16>(model, 0x48);
    const u8 *part = at<const u8 *>(model, 0x7c);
    u32 seen = 0;
    for (u32 i = 0; i < n; ++i, part += 0x14)
        if ((at<u32>(part, 0) & mask) && ++seen == nth) return i;
    return 0xffffffff;
}

u32 __cdecl Model_CountParts(const u8 *holder, u32 mask) {
    const u8 *model = model_of(holder);
    const u8 *part = at<const u8 *>(model, 0x7c);
    u32 count = 0;
    for (u32 n = at<u16>(model, 0x48); n; --n, part += 0x14)
        if (at<u32>(part, 0) & mask) ++count;
    return count;
}
}
