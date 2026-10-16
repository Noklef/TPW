#include "objectmsg.h"

#include <cstdint>

#include "../core/vcall.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

// One ring: the temp dword at base, the data after it, then the index, the length and the wrapped byte.
void Advance(u8 *object, u32 base) {
    const s32 length = at<s32>(object, base + 0x80);
    const u32 temp = at<u32>(object, base);
    const s32 next = at<s32>(object, base + 0x7c) + 1;
    at<s32>(object, base + 0x7c) = next;
    if (next >= length) {
        at<u32>(object, base + 0x7c) = 0;
        at<u8>(object, base + 0x84) = 1;
    }
    at<u32>(object, base + 4 * at<u32>(object, base + 0x7c) + 4) = temp;
    at<u32>(object, base) = 0;
}

}  // namespace

extern "C" {

u32 __fastcall Object_HandleDayMessage(u8 *object, void *, u8 *message) {
    const u32 type = vcall<u32>(message, 0);
    if (type == 0xb) {
        static const u32 bases[6] = {0x1a8, 0x230, 0x2b8, 0x70, 0xf8, 0x340};
        for (u32 base : bases)
            Advance(object, base);
        return 0;
    }
    if (type == 0x1b && at<u16>(message, 8) == at<u16>(object, 0x5e)) {  // (the Debug_Printf is left out)
        at<u16>(object, 0x5e) = 0;
        at<u32>(object, 0x60) = 0;
        return 0;
    }
    return type;
}

}  // extern "C"
