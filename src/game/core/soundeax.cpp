#include "soundeax.h"

#include <cstdint>

#include "vcall.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

extern "C" {

u32 *__fastcall EaxBuffer_QueryAll(u8 *buffer, void *, u32 *out, u32) {
    u32 data[0xf];   // (the size returned lands on the data's first dword, sic)
    u8 *const set = at<u8 *>(buffer, 4);
    const u32 result = comcall<u32>(set, 0xc, 0x0070aa50u, 1u, 0u, 0u, data, 0x38u, data);
    *out = result ? 0xffffffff : 0;
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
