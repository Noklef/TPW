#include "small56.h"

#include <cstdint>

#include "../model/slots.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

}  // namespace

extern "C" {

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

}  // extern "C"
