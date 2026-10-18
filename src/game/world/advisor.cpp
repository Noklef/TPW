#include "advisor.h"

#include <cstdint>

#include "../model/parts.h"
#include "../model/slots.h"
#include "../motion/animator.h"
#include "../render/particles.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

}  // namespace

extern "C" {

u32 __cdecl Table768fb8_Lookup(u32 id) {
    for (u32 entry = 0x00768fb8; global<u32>(entry) != 0x270f; entry += 0x20) {
        if (global<u32>(entry) == id)
            return global<u32>(entry + 0x1c);
    }
    return 0x17f;
}

void __cdecl Advisor_Init() {
    for (u32 offset = 0; offset < 0x168; offset += 4)
        global<u32>(0x00f79660 + offset) = 0;
    global<s32>(0x00f79770) = 5;
    global<u32>(0x00f7978c) = 0xffffffff;
    for (u32 slot = 0; slot < 2; slot++) {
        const u8 *holder = ModelSlot_Holder(slot);
        if (!holder)
            continue;
        for (s32 j = 0; j < global<s32>(0x00f79770); j++)
            global<u32>(0x00f796a0 + 0x14 * slot + 4 * j) = Model_FindMouthPart(holder, 0, j + 1);
        global<u32>(0x00f79698 + 4 * slot) = Model_FindMouthPart(holder, 0, -1);
    }
    const u8 *holder = ModelSlot_Holder(1);
    if (!holder)
        holder = ModelSlot_Holder(0);
    if (!holder)
        return;
    for (u32 k = 0; k < 20; k++)
        global<u32>(0x00f796c8 + 4 * k) = Animator_MotionEndMs(holder, 5, k);
    global<u32>(0x00f796fc) = 0;
    ParticlePools_Reset();
}
}
