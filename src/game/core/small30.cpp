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

u32 __cdecl Code_Remap46b460(u32 code) {
    static const u16 kLetters[0x1b] = {0x3b,  0x5e9, 0x5e0, 0x5d1, 0x5d2, 0x5e7, 0x5db, 0x5e2, 0x5d9, 0x5df, 0x5d7, 0x5dc, 0x5da, 0x5e6,
                                       0x5de, 0x5dd, 0x5e4, 0x2f,  0x5e8, 0x5d3, 0x5d0, 0x5d5, 0x5d4, 0x27,  0x5e1, 0x5d8, 0x5d6};
    u32 mapped = code & 0xffff;
    switch (mapped) {
    case 0x27: mapped = 0x2c; break;
    case 0x2c: mapped = 0x5ea; break;
    case 0x2e: mapped = 0x5e5; break;
    case 0x2f: mapped = 0x2e; break;
    case 0x3b: mapped = 0x5e3; break;
    default:
        if (mapped >= 0x60 && mapped <= 0x7a)
            mapped = kLetters[mapped - 0x60];
    }
    return (code & 0xffff0000u) | mapped;
}

}  // extern "C"
