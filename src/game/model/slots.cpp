#include "slots.h"

#include <cstdint>

#include "../motion/animator.h"
#include "parts.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) {
    return *reinterpret_cast<T *>((uintptr_t)p + offset);
}

u8 *&holder_of(u32 slot) { return *reinterpret_cast<u8 **>((uintptr_t)(0x00790398 + slot * 0xb0 + 4)); }

// The face of the slot's part with key and flag 0x400, or null.
u32 *part_flags(u32 key, u32 slot) {
    const u32 part = Model_FindPartByKey4(holder_of(slot), 0x400, key);
    if (part == 0xffffffff) return nullptr;
    const u8 *const model = at<const u8 *>(at<const u8 *>(holder_of(slot), 8), 4);
    const u32 index = at<u16>(model, 0x46) + part, first = at<u16>(model, 0x44);
    return reinterpret_cast<u32 *>(index < first ? at<u8 *>(model, 0x70) + index * 0xa0
                                                 : at<u8 *>(model, 0x74) + (index - first) * 0x58);
}

}  // namespace

extern "C" {

void __cdecl ModelSlot_Animate(u32 slot) {
    if (u8 *const holder = holder_of(slot)) Animator_Update(holder, reinterpret_cast<const u8 *>(1), 0);
}

void __cdecl ModelSlot_ClearPartFlag(u32 key, u32 slot) {
    if (u32 *const flags = part_flags(key, slot)) *flags &= ~0x10u;
}

void __cdecl ModelSlot_SetPartFlag(u32 key, u32 slot) {
    if (u32 *const flags = part_flags(key, slot)) *flags |= 0x10;
}

u8 *__cdecl ModelSlot_Holder(u32 slot) { return holder_of(slot); }
}
