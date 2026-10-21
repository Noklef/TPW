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

}  // extern "C"
