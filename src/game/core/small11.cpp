#include "small11.h"

#include <cstdint>

#include "../world/objectdefs.h"
#include "gameheap.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

}  // namespace

extern "C" {

void __fastcall Slots_AddIfMatch(u8 *object, void *, u32 id, u32 amount, u32 kind, u32 extra) {
    const s32 index = at<s32>(object, 0x3cc);
    if (index < 0 || index >= 20)
        return;
    u8 *slot = object + 0xc + 48 * (u32)index;
    if (!at<u8>(object, 0x3d9))
        return;
    if (id != at<u32>(slot, 0))
        return;
    if ((u16)kind != at<u16>(slot, 0xc))
        return;
    const u16 limit = at<u16>(object, 0x3e0);
    if (limit && (u16)extra != limit)
        return;
    at<u32>(object, 0x3dc) += amount;
}

u32 __fastcall Slots_GetValue(const u8 *object, void *, u32 slot) {
    const s32 entry = at<s32>(object, 0x1414 + 16 * slot);
    if (entry < 0)
        return 0;
    if (slot == 4) {
        const u32 index = at<u32>(object, 0x1454);
        const u8 *record = object + 20 * index;
        const u8 *def = ObjectDefs_Find(reinterpret_cast<u8 *>(0x007890a0), nullptr, at<u16>(record, 0xc));
        return at<u32>(def, 0x40 * at<u8>(record, 0x15) + 0x1b8);
    }
    const u8 *record = object + 20 * (u32)entry + 0xc;
    u8 *controls = global<u8 *>(0x0080239c) + 0x1d943c;
    return at<u32>(ObjectControls_FindUncached(controls, nullptr, at<u16>(record, 0), 0), 4);
}

u32 *__fastcall List_EraseRangeCounted(u8 *object, void *, u32 *out, u8 *first, u8 *last) {
    while (first != last) {
        u8 *node = first;
        first = at<u8 *>(node, 0);
        at<u32>(at<u8 *>(node, 4), 0) = at<u32>(node, 0);
        at<u32>(at<u8 *>(node, 0), 4) = at<u32>(node, 4);
        u8 *buffer = at<u8 *>(node, 0xc);
        if (buffer) {
            const u8 uses = buffer[-1];
            if (uses == 0 || uses == 0xff)
                GameHeap_Free(buffer - 2);
            else
                buffer[-1] = (u8)(uses - 1);
        }
        at<u32>(node, 0xc) = 0;
        at<u32>(node, 0x10) = 0;
        at<u32>(node, 0x14) = 0;
        GameHeap_Free(node);
        at<u32>(object, 8) -= 1;
    }
    *out = (u32)(uintptr_t)first;
    return out;
}

}  // extern "C"
