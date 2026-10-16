#include "spritequeue.h"

#include <cstdint>

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

extern "C" {

u32 __fastcall SpriteQueue_Add(u8 *queue, void *, u32 frame) {
    s32 n = at<s32>(queue, 0x2404);
    if (n >= 0x800)
        return 0;
    reinterpret_cast<u32 *>(queue)[n] = frame;
    at<s32>(queue, 0x2404) = at<s32>(queue, 0x2404) + 1;
    return 1;
}

u32 __fastcall SpriteQueue_Add2(u8 *queue, void *, u32 frame) {
    s32 n = at<s32>(queue, 0x2408);
    if (n >= 0x800)
        return 0;
    reinterpret_cast<u32 *>(queue + 0x2000)[n] = frame;
    // re-read: the entry may have been the count itself
    at<s32>(queue, 0x2408) = at<s32>(queue, 0x2408) + 1;
    return 1;
}
}
