#include "parts.h"

#include <cstdint>

#include "../core/crt.h"

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

void __cdecl Model_MarkFlag80Faces(const u8 *const *holder) {
    const u32 n = at<u16>(at<const u8 *>(*holder, 4), 0x48);
    for (u32 i = 0; i < n; ++i) {
        const u8 *model = at<const u8 *>(*holder, 4);
        if (!(at<u8>(at<const u8 *>(model, 0x7c), i * 0x14) & 0x80)) continue;
        at<u32>(face(model, i), 0) |= 0x80000010;
        at<u32>(at<u8 *>(*holder, 0x28), 0) |= 2;
    }
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

u32 __cdecl Model_FindMouthPart(const u8 *model, u32, s32 part) {
    const u8 *data = at<const u8 *>(at<const u8 *>(model, 8), 4);
    const u32 count = at<u16>(data, 0x42);
    if (count == 0)
        return 0;
    const char *wanted;
    switch (part) {
    case 1: wanted = reinterpret_cast<const char *>(0x0074cf08); break;
    case 2: wanted = reinterpret_cast<const char *>(0x0074cf18); break;
    case 3: wanted = reinterpret_cast<const char *>(0x0074cf24); break;
    case 4: wanted = reinterpret_cast<const char *>(0x0074cefc); break;
    case 5: wanted = reinterpret_cast<const char *>(0x0074cee4); break;
    case -1: wanted = reinterpret_cast<const char *>(0x0074cef0); break;
    default: return 0;
    }
    for (u32 i = 0; i < count; ++i) {
        const char *name = at<const char *>(at<const u8 *>(data, 0x70), 0xa0 * i + 0x54);
        if (Crt().stricmp(name, wanted) == 0)
            return i;
    }
    return 0;
}

void __cdecl Model_FindSignEntries(u8 *model, u32 *out1, u32 *out2) {
    u8 *entry = at<u8 *>(model, 0x54);
    for (u32 left = at<u16>(model, 0x36); left != 0; --left, entry += 0x10) {
        const u32 flags = at<u32>(entry, 0);
        const u32 offset = at<u32>(model, 0x50) + (u32)(((s32)((uintptr_t)entry - at<u32>(model, 0x54))) >> 4) * 8;
        if (((flags & 2) && (flags & 8)) || Crt().stricmp(at<const char *>(entry, 0xc), "sign1.tga") == 0)
            *out1 = offset;
        const u32 flags2 = at<u32>(entry, 0);
        if (((flags2 & 2) && (flags2 & 0x10)) || Crt().stricmp(at<const char *>(entry, 0xc), "sign2.tga") == 0)
            *out2 = offset;
    }
}
}
