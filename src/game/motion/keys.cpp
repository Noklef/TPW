// Key search for .md2 motion tracks: reimplementation of 0x470b60 and 0x470c90.
#include "keys.h"

namespace {

template <typename T>
T field(const void *base, u32 offset) {
    return *reinterpret_cast<const T *>(static_cast<const u8 *>(base) + offset);
}

u32 whole_frame(const void *state) { return (u32)(long long)field<float>(state, 0x20); }  // __ftol

}  // namespace

extern "C" s32 __cdecl Motion_FindKeys(const void *state, const u16 *keys, u32 count, s32 kind, s32 loop, u32 *i,
                                       u32 *j, float *t) {
    u32 stride;
    if (kind == 1)
        stride = 4;
    else if (kind == 2)
        stride = 20;
    else if (kind == 4)
        stride = 16;
    else
        return 0;
    auto frame_of = [&](u32 k) { return (u32) * reinterpret_cast<const u16 *>(reinterpret_cast<const u8 *>(keys) + k * stride); };
    const u32 whole = whole_frame(state);
    bool found = false;
    u32 last = 0, f0 = 0;
    for (u32 k = 0; k < count && frame_of(k) <= whole; ++k) {
        last = k;
        f0 = frame_of(k);
        found = true;
    }
    if (!found)
        return 0;
    u32 next, f1;
    if (last == count - 1) {
        if (loop) {
            next = 0;
            f1 = frame_of(0);
        } else {
            next = last;
            f1 = frame_of(last) + 1;
        }
    } else {
        next = last + 1;
        f1 = frame_of(next);
    }
    // frames are below 2^17: exact as floats
    *t = (field<float>(state, 0x20) - (float)f0) / ((float)f1 - (float)f0);
    *i = last;
    *j = next;
    return 1;
}

extern "C" void __cdecl Motion_ApplyMaterial(void *model, const void *table, const void *state) {
    const u32 whole = whole_frame(state);
    const MaterialAnim *anim = field<const MaterialAnim *>(table, 0x30);
    u8 *materials = field<u8 *>(model, 0x50);
    for (u32 n = field<u16>(table, 0x18); n != 0; --n, ++anim) {
        u8 *record = materials + 8 * anim->material;
        for (u32 k = anim->keys; k != 0; --k) {
            const u32 key = anim->key[k - 1];
            if ((key & 0xffff) <= whole) {
                const s16 value = (s16)(key >> 16);
                if (*reinterpret_cast<s16 *>(record + 2) != value) {
                    record[1] |= 4;
                    *reinterpret_cast<s16 *>(record + 2) = value;
                }
                break;
            }
        }
    }
}
