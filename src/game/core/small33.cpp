#include "small33.h"

#include <cstdint>

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

u32 Fire(u8 *table, u32 a, u32 b, u32 handlerAt, u16 flag) {
    if (at<u16>(table, 6) == 0)
        return 0;
    s32 found = -1;
    const s32 count = at<s16>(table, 4);
    for (s32 i = 0; i < count && found < 0; ++i) {
        const u8 *entry = at<u8 *>(table, 0) + 0x14 * i;
        if (at<u16>(entry, 2) == (u16)a && at<u16>(entry, 4) == (u16)b)
            found = i;
    }
    if (found < 0)
        return 0;
    u8 *entry = at<u8 *>(table, 0) + 0x14 * found;
    u32 result = 0;
    if (void *handler = at<void *>(entry, handlerAt))
        result = reinterpret_cast<u32(__fastcall *)(void *)>(handler)(handler);
    at<u16>(at<u8 *>(table, 0) + 0x14 * found, 6) = flag;
    return result;
}

}  // namespace

extern "C" {

u32 __fastcall Actions_FireDown(u8 *table, void *, u32 a, u32 b) { return Fire(table, a, b, 0x10, 1); }

u32 __fastcall Actions_FireUp(u8 *table, void *, u32 a, u32 b) { return Fire(table, a, b, 0xc, 0); }

}  // extern "C"
