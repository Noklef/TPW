#include "small30.h"

#include <cstdint>

#include "crt.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

extern "C" {

void __cdecl Model_FindSignEntries(u8 *model, u32 *out1, u32 *out2) {
    u8 *entry = at<u8 *>(model, 0x54);
    for (u32 left = at<u16>(model, 0x36); left != 0; --left, entry += 0x10) {
        const u32 flags = at<u32>(entry, 0);
        const u32 offset = at<u32>(model, 0x50) + (u32)(((s32)((uintptr_t)entry - at<u32>(model, 0x54))) >> 4) * 8;
        if (((flags & 2) && (flags & 8)) || Crt().stricmp(at<const char *>(entry, 0xc), "sign1.tga") == 0)
            *out1 = offset;
        const u32 flags2 = at<u32>(entry, 0);
        if (((flags2 & 2) && (flags2 & 0x10)) || Crt().stricmp(at<const char *>(entry, 0xc), "sign2.tga") == 0)
            *out2 = offset;
    }
}

}  // extern "C"
