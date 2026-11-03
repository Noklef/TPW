#include "wstringmap.h"

#include <cstdint>

#include "crt.h"
#include "stdtree.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

// -1, 0 or 1: the 16-bit characters up to the shorter length, then the lengths.
s32 Compare(const u16 *a, u32 aLength, const u16 *b, u32 bLength) {
    for (u32 n = aLength < bLength ? aLength : bLength; n != 0; --n, ++a, ++b) {
        if (*a != *b)
            return *a < *b ? -1 : 1;
    }
    if (aLength < bLength)
        return -1;
    return aLength != bLength ? 1 : 0;
}

// The second wide string copy's order (case-insensitive): _wcsnicmp up to the shorter length, then the lengths.
s32 CompareI(const u16 *a, u32 aLength, const u16 *b, u32 bLength) {
    const s32 r = Crt().wcsnicmp(a, b, aLength < bLength ? aLength : bLength);
    if (r)
        return r;
    if (aLength < bLength)
        return -1;
    return aLength != bLength;
}

const u16 *NullStrI(const u16 *chars) { return chars ? chars : reinterpret_cast<const u16 *>(0x00700624); }

u8 *LowerBoundI(u8 *set, const u8 *key, u32 sentinel) {
    u8 *result = at<u8 *>(set, 4);
    u8 *node = at<u8 *>(result, 4);
    if (node == global<u8 *>(sentinel))
        return result;
    do {
        if (CompareI(at<const u16 *>(node, 0x10), at<u32>(node, 0x14), NullStrI(at<const u16 *>(key, 4)), at<u32>(key, 8)) < 0) {
            node = at<u8 *>(node, 8);
        } else {
            result = node;
            node = at<u8 *>(node, 0);
        }
    } while (node != global<u8 *>(sentinel));
    return result;
}

u32 *FindI(u8 *set, u32 *out, const u8 *key, u8 *found) {
    u8 *const head = at<u8 *>(set, 4);
    if (found != head &&
        CompareI(at<const u16 *>(key, 4), at<u32>(key, 8), NullStrI(at<const u16 *>(found, 0x10)), at<u32>(found, 0x14)) >= 0) {
        *out = (u32)(uintptr_t)found;
        return out;
    }
    *out = (u32)(uintptr_t)head;
    return out;
}

}  // namespace

extern "C" {

u8 *__fastcall ChatIgnoreSet_LowerBound(u8 *set, void *, const u8 *key) { return LowerBoundI(set, key, 0x007ced34); }

u8 *__fastcall ChatBuddySet_LowerBound(u8 *set, void *, const u8 *key) { return LowerBoundI(set, key, 0x007ced2c); }

u8 *__fastcall ChatBuddySet_UpperBound(u8 *set, void *, const u8 *key) {
    u8 *result = at<u8 *>(set, 4);
    u8 *node = at<u8 *>(result, 4);
    const u8 *const nil = global<const u8 *>(0x007ced2c);
    while (node != nil) {
        if (CompareI(at<const u16 *>(key, 4), at<u32>(key, 8), NullStrI(at<const u16 *>(node, 0x10)), at<u32>(node, 0x14)) < 0) {
            result = node;
            node = at<u8 *>(node, 0);
        } else {
            node = at<u8 *>(node, 8);
        }
    }
    return result;
}

u32 *__fastcall ChatIgnoreSet_Find(u8 *set, void *, u32 *out, const u8 *key) {
    return FindI(set, out, key, ChatIgnoreSet_LowerBound(set, nullptr, key));
}

u32 *__fastcall ChatBuddySet_Find(u8 *set, void *, u32 *out, const u8 *key) {
    return FindI(set, out, key, ChatBuddySet_LowerBound(set, nullptr, key));
}

u32 *__fastcall ChatBuddySet_LowerBoundOut(u8 *set, void *, u32 *out, const u8 *key) {
    *out = (u32)(uintptr_t)ChatBuddySet_LowerBound(set, nullptr, key);
    return out;
}

u8 *__fastcall WStringMap_LowerBound(u8 *tree, void *, const u8 *key) {
    u8 *result = at<u8 *>(tree, 4);
    u8 *const sentinel = global<u8 *>(0x007ced24);
    u8 *node = at<u8 *>(result, 4);
    if (node == sentinel)
        return result;
    const u32 length = at<u32>(key, 8);
    do {
        const u16 *chars = at<const u16 *>(key, 4);
        if (!chars)
            chars = reinterpret_cast<const u16 *>(0x006fd6c4);
        if (Compare(at<const u16 *>(node, 0x10), at<u32>(node, 0x14), chars, length) < 0) {
            node = at<u8 *>(node, 8);
        } else {
            result = node;
            node = at<u8 *>(node, 0);
        }
    } while (node != sentinel);
    return result;
}

u8 *__fastcall WStringMap_UpperBound(u8 *tree, void *, const u8 *key) {
    u8 *result = at<u8 *>(tree, 4);
    u8 *const sentinel = global<u8 *>(0x007ced24);
    u8 *node = at<u8 *>(result, 4);
    if (node == sentinel)
        return result;
    const u32 length = at<u32>(key, 8);
    do {
        const u16 *chars = at<const u16 *>(node, 0x10);
        if (!chars)
            chars = reinterpret_cast<const u16 *>(0x006fd6c4);
        if (Compare(at<const u16 *>(key, 4), length, chars, at<u32>(node, 0x14)) < 0) {
            result = node;
            node = at<u8 *>(node, 0);
        } else {
            node = at<u8 *>(node, 8);
        }
    } while (node != sentinel);
    return result;
}

u32 *__fastcall WStringMap_Find(u8 *map, void *, u32 *out, const u8 *key) {
    u8 *const found = WStringMap_LowerBound(map, nullptr, key);
    u8 *const head = at<u8 *>(map, 4);
    if (found != head) {
        const u16 *chars = at<const u16 *>(found, 0x10);
        if (!chars)
            chars = reinterpret_cast<const u16 *>(0x006fd6c4);
        // the key against the node's string
        if (Compare(at<const u16 *>(key, 4), at<u32>(key, 8), chars, at<u32>(found, 0x14)) >= 0) {
            *out = (u32)(uintptr_t)found;
            return out;
        }
    }
    *out = (u32)(uintptr_t)head;
    return out;
}

u32 __fastcall WStringMap_EraseKey(u8 *map, void *, const u8 *key) {
    u8 *const last = WStringMap_UpperBound(map, nullptr, key);
    u8 *const first = WStringMap_LowerBound(map, nullptr, key);
    u32 n = 0;
    for (u8 *it = first; it != last; StdTree_IncD(&it))
        ++n;
    u8 *out;
    StdTree_EraseRangeD(map, nullptr, &out, first, last);
    return n;
}

u32 __fastcall ChatBuddySet_EraseKey(u8 *set, void *, const u8 *key) {
    u8 *const last = ChatBuddySet_UpperBound(set, nullptr, key);
    u8 *const first = ChatBuddySet_LowerBound(set, nullptr, key);
    u32 n = 0;
    for (u8 *it = first; it != last; StdTree_IncC(&it))
        ++n;
    u8 *out;
    StdTree_EraseRangeC(set, nullptr, &out, first, last);
    return n;
}

}  // extern "C"
