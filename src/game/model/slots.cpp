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

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

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

void __cdecl ModelSlot_ApplySelection(s32 kind) {
    const u32 slot = global<u32>(0x00f79674);
    // the byte table 0x598da4 sends kinds -1..14 to: loop, clear, set (1, 2, 4, 5, 6, 7, 9, 10, 11, 13), set 0x15 (14) or nothing
    switch (kind) {
    case 0:
        ModelSlot_ClearPartFlag(0x13, slot);
        ModelSlot_ClearPartFlag(0x14, slot);
        break;
    case 1: case 2: case 4: case 5: case 6: case 7: case 9: case 10: case 11: case 13:
        ModelSlot_SetPartFlag(0x13, slot);
        ModelSlot_SetPartFlag(0x14, slot);
        break;
    case 14:
        ModelSlot_SetPartFlag(0x15, slot);
        break;
    case -1:
        for (u32 address = 0x00f797b4; address < 0x00f797c4; address += 4) {
            const u32 clear = global<u32>(address - 0x10);
            if (clear && !(global<u32>(0x00f79660) & 0x10000))
                ModelSlot_ClearPartFlag(clear, global<u32>(0x00f79674));
            const u32 set = global<u32>(address);
            if (set)
                ModelSlot_SetPartFlag(set, global<u32>(0x00f79674));
        }
        break;
    default:
        break;
    }
    if (kind != 0 && !(global<u32>(0x00f79660) & 0x10000))
        ModelSlot_ClearPartFlag((u32)kind, global<u32>(0x00f79674));
}
}
