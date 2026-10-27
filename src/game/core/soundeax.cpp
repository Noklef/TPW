#include "soundeax.h"

#include <cstdint>

#include "vcall.h"
#include "waveformat.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

extern "C" {

u32 *__fastcall EaxBuffer_Open(u8 *buffer, void *, u32 *out, u8 *sound, u32 rate) {
    u32 holder[7];
    at<u8>(holder, 0) = 1;
    at<u8>(holder, 1) = 0x10;
    at<u16>(holder, 2) = static_cast<u16>(rate);
    at<u16>(holder, 4) = 0;
    holder[6] = 0;
    u32 desc[5] = {0x14, 0x10, 0x40, 0, 0};
    desc[4] = static_cast<u32>(reinterpret_cast<uintptr_t>(WaveFormat_Build(reinterpret_cast<u8 *>(holder))));
    if (sound && static_cast<s32>(comcall<u32>(sound, 0xc, desc, buffer + 8, 0u)) >= 0 &&
        static_cast<s32>(comcall<u32>(at<u8 *>(buffer, 8), 0, 0x00707fa0u, buffer)) >= 0 &&
        static_cast<s32>(comcall<u32>(at<u8 *>(buffer, 0), 0, 0x00707fe0u, buffer + 4)) >= 0) {
        rate = 0;   // (the support's out is the rate's own slot, sic)
        if (!comcall<u32>(at<u8 *>(buffer, 4), 0x14, 0x0070aa50u, 1u, &rate) && (rate & 1)) {
            *out = 0;
            return out;
        }
    }
    EaxBuffer_Release(buffer);
    *out = 0xffffffff;
    return out;
}

void __fastcall EaxBuffer_Release(u8 *buffer) {
    static const u32 kOrder[3] = {4, 0, 8};
    for (u32 offset : kOrder) {
        u8 *const object = at<u8 *>(buffer, offset);
        if (object) {
            comcall<u32>(object, 8);
            at<u8 *>(buffer, offset) = nullptr;
        }
    }
}

u32 *__fastcall EaxBuffer_QueryAll(u8 *buffer, void *, u32 *out, u32) {
    u32 data[0xf];   // (the size returned lands on the data's first dword, sic)
    u8 *const set = at<u8 *>(buffer, 4);
    const u32 result = comcall<u32>(set, 0xc, 0x0070aa50u, 1u, 0u, 0u, data, 0x38u, data);
    *out = result ? 0xffffffff : 0;
    return out;
}

u32 *__fastcall EaxBuffer_SetProperty0b(u8 *buffer, void *, u32 *out, u32 value) {
    u8 *const set = at<u8 *>(buffer, 4);
    if (!set) {
        *out = 0xffffffff;
        return out;
    }
    *out = comcall<u32>(set, 0x10, 0x0070aa50u, 0xbu, 0u, 0u, &value, 4u);
    return out;
}

u32 *__fastcall EaxBuffer_SetAll(u8 *buffer, void *, u32 *out, const u32 *properties) {
    u8 *const set = at<u8 *>(buffer, 4);
    if (!set) {
        *out = 0xffffffff;
        return out;
    }
    static const u8 kOrder[13] = {3, 4, 11, 5, 6, 7, 8, 9, 10, 0, 1, 2, 12};
    u32 data[14];
    for (u32 i = 0; i < 13; ++i) data[i] = properties[kOrder[i]];
    data[13] = 9;
    *out = comcall<u32>(set, 0x10, 0x0070aa50u, 1u, 0u, 0u, data, 0x38u);
    return out;
}

void __fastcall EaxListener_SetAll(u8 *self, void *, u32 key, const u32 *properties) {
    u8 *buffer;   // (left as it was when the slot does not set it, sic)
    vcall<void>(self, 0x10, key, &buffer);
    if (!buffer) return;
    u8 *set;
    if (static_cast<s32>(comcall<u32>(buffer, 0, 0x00707fe0u, &set)) < 0) return;
    u32 data[13];
    for (u32 i = 0; i < 12; ++i) data[i] = properties[i];
    data[12] = 3;
    comcall<u32>(set, 0x10, 0x0070aa60u, 1u, 0u, 0u, data, 0x34u);
    comcall<u32>(set, 8);
}

}  // extern "C"
