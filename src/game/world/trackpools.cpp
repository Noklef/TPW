#include "trackpools.h"

#include <cstdint>

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

// The first record of the pool that is not in use, among count of them.
u8 *FirstFree(u32 poolAt, u32 size, s32 count, bool storeIndex) {
    u8 *record = global<u8 *>(poolAt);
    if (!record)
        return nullptr;
    for (s32 i = 0; i < count; ++i, record += size) {
        if (!(record[0] & 1)) {
            if (storeIndex)
                at<s32>(record, 4) = i;
            return record;
        }
    }
    return nullptr;
}

}  // namespace

extern "C" {

u8 *__cdecl Buoy_FindFree() { return FirstFree(0x00877b78, 0x1c, 0x400, true); }

u8 *__cdecl Buoy_Alloc(u32 a, u32 b) {
    u8 *buoy = FirstFree(0x00877b78, 0x1c, 0x400, true);
    if (!buoy)
        return nullptr;
    global<s32>(0x00877b7c) += 1;
    at<u32>(buoy, 0) = 1;
    at<u32>(buoy, 8) = a;
    at<u32>(buoy, 0xc) = b;
    at<u32>(buoy, 0x10) = 0;
    at<u32>(buoy, 0x14) = 0;
    at<u32>(buoy, 0x18) = 0;
    return buoy;
}

u8 *__cdecl CollNode_FindFree() { return FirstFree(0x00877b70, 0x20, 0x800, false); }

u8 *__cdecl CollNode_Alloc(u32 a, u32 b, u32 c) {
    u8 *node = FirstFree(0x00877b70, 0x20, 0x800, false);
    if (!node)
        return nullptr;
    global<s32>(0x00877b74) += 1;
    at<u32>(node, 4) = a;
    at<u32>(node, 0) = 0x1f;
    at<u32>(node, 8) = b;
    at<u32>(node, 0x10) = c;
    at<u32>(node, 0xc) = c;
    for (u32 off = 0x14; off < 0x20; off += 4)
        at<u32>(node, off) = 0;
    return node;
}

}  // extern "C"
