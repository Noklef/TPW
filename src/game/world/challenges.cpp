#include "challenges.h"

#include <cstdint>

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

extern "C" {

void __fastcall Challenges_DropOrphans(u8 *challenges) {
    u8 *const list = challenges + 0xc;
    for (s32 i = 0; i < 0x14; ++i) {
        u8 *const c = list + i * 0x30;
        if (at<u32>(c, 0) == 0) continue;
        const u32 needs = at<u32>(c, 0x18);
        if (needs == 0) continue;
        bool found = false;
        for (s32 j = 0; j < 0x14; ++j) {
            if (j == i) continue;
            const u32 id = at<u32>(list, j * 0x30);
            if (id != 0 && id == needs) {
                found = true;
                break;
            }
        }
        if (!found) at<u32>(c, 0) = 0;
    }
}

u8 *__fastcall Challenges_Current(u8 *challenges) {
    const s32 index = at<s32>(challenges, 0x3cc);
    if (index < 0 || index >= 0x14) return nullptr;
    return challenges + 0xc + index * 0x30;
}

const u8 *__fastcall BalanceChallenges_GetSchema(const u8 *, void *, u32 index) {
    return reinterpret_cast<const u8 *>(0x00739b98) + index * 0x3c;
}
}
