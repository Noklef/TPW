#include "challenges.h"

#include <cstdint>

#include "../core/savegame.h"
#include "things.h"

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
u32 __fastcall Challenge_Serialize(u8 *challenge, void *, Stream *stream, u32 save, s32 version, u32) {
    static const SaveSpec kFields[] = {{0x0, 4, 0x0075a034},  {0x4, 4, 0x0075a168},  {0x8, 4, 0x0075a15c},  {0xc, 2, 0x0075a150},  {0xe, 2, 0x0075a144},
                                       {0x10, 4, 0x0075a130}, {0x14, 4, 0x0075a128}, {0x18, 4, 0x0075a10c}, {0x1c, 1, 0x0075a0fc}, {0x20, 4, 0x0075a0e8},
                                       {0x24, 4, 0x0075a0d8}, {0x28, 4, 0x0075a0c4}, {0x2c, 1, 0x0075a0b8}, {0x1d, 1, 0x0075a0a8}, {0x2e, 2, 0x0075a094}};
    if (version < 0 || version > 1)
        return 0;
    return Save_Fields(stream, save, challenge, kFields, 15) ? 0 : 1;
}

u32 __fastcall Challenges_Serialize(u8 *challenges, void *, Stream *stream, u32 save, s32 version, u32 unused) {
    static const SaveSpec kFields[] = {{0x3cc, 4, 0x0075a2b0}, {0x3d0, 8, 0x0075a298}, {0x3d8, 1, 0x0075a284}, {0x3d9, 1, 0x0075a274},
                                       {0x3da, 1, 0x0075a260}, {0x3dc, 4, 0x0075a258}, {0x3e0, 2, 0x0075a248}};
    Thing_Serialize(challenges, nullptr, stream, save, version, unused);  // sic: the result is ignored
    if (version < 0 || version > 1)
        return 0;
    for (u32 i = 0; i < 0x14; ++i)
        Challenge_Serialize(challenges + 0xc + i * 0x30, nullptr, stream, save, version, unused);  // sic: the results are ignored
    return Save_Fields(stream, save, challenges, kFields, 7) ? 0 : 1;
}
}
