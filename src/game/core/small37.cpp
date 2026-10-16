#include "small37.h"

#include <cstdint>

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

}  // namespace

extern "C" {

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

}  // extern "C"
