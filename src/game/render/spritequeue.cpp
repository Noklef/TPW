#include "spritequeue.h"

#include <cstdint>

#include "../core/gameheap.h"
#include "../render/engineerrors.h"
#include "../gfx/texture.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

extern "C" {

u32 __fastcall SpriteQueue_Ready(u8 *setPlus20) {
    u8 *set = setPlus20 - 0x20;
    const u32 stampA = *reinterpret_cast<u32 *>(0x008bcc40), stampB = *reinterpret_cast<u32 *>(0x008bcc44);
    if (at<u32>(set, 0xc) == stampA && at<u32>(set, 0x10) == stampB)
        return 1;
    u8 *queue = *reinterpret_cast<u8 **>(0x008c47a0);
    u32 result = 0;
    if (at<s32>(queue, 0x240c) < 0x80) {
        at<u32>(queue, 0x2200 + at<s32>(queue, 0x240c) * 4) = (u32)(uintptr_t)set;
        at<s32>(queue, 0x240c) = at<s32>(queue, 0x240c) + 1;
        result = 1;
    }
    at<u32>(set, 0xc) = stampA;
    at<u32>(set, 0x10) = stampB;
    return result;
}

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

u32 __cdecl SpriteQueue_Free() {
    u8 *const queue = *reinterpret_cast<u8 **>(0x008c47a0);
    if (!queue)
        return 0;
    if (u8 *const textures = at<u8 *>(queue, 0x2400)) {
        Texture_DestroyArray(textures);
        at<u32>(queue, 0x2400) = 0;
    }
    GameHeap_Free(queue);
    *reinterpret_cast<u8 **>(0x008c47a0) = nullptr;
    return 1;
}

u32 __cdecl SpriteQueue_Create() {
    u8 *&slot = *reinterpret_cast<u8 **>(0x008c47a0);
    if (slot)
        return 0;
    u8 *const queue = static_cast<u8 *>(GameHeap_Alloc(0x2410));
    if (queue) {
        at<u32>(queue, 0x2404) = 0;
        at<u32>(queue, 0x2408) = 0;
        at<u32>(queue, 0x2400) = 0;
        at<u32>(queue, 0x240c) = 0;
    }
    slot = queue;
    if (!queue) {
        EngineErrors_Push(reinterpret_cast<u8 *>((uintptr_t)0x00879ad0), nullptr, 3, 0xb0754000);
        return 0;
    }
    return 1;
}
}
