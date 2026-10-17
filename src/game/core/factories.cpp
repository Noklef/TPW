#include "factories.h"

#include <cstdint>
#include "gameheap.h"
#include "misc.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

extern "C" {

u32 *__cdecl Class7037c0_Create(u32 *result, u8 **object) {
    if (!object) {
        *result = 0x80004003;
        return result;
    }
    u8 *block = static_cast<u8 *>(GameHeap_Alloc(0x18));
    if (block) {
        Class703770_Ctor(block);
        at<u32>(block, 8) = 0;
        at<u32>(block, 0) = 0x007037c0;
    }
    if (!block) {
        *result = 0xcffe0100;
        return result;
    }
    *object = block;
    *result = 0;
    return result;
}

u32 *__cdecl Class703750_Create(u32 *result, u8 **object) {
    if (!object) {
        *result = 0x80004003;
        return result;
    }
    u8 *block = static_cast<u8 *>(GameHeap_Alloc(0x14));
    if (block) {
        Class703770_Ctor(block);
        at<u32>(block, 8) = 0;
        at<u32>(block, 0xc) = 0;
        at<u32>(block, 0) = 0x00703750;
    }
    if (!block) {
        *result = 0xcffe0100;
        return result;
    }
    *object = block;
    *result = 0;
    return result;
}
}  // extern "C"
