#include "idmaps.h"

#include <cstdint>

namespace {

// The offsets from the base id of the kinds 1 to 0x21 (index 0 is unused).
const u8 kOffsets[0x22] = {0,    0,    1,    2,    3,    4,    5,    6,    7,    0xb,  0xc,  0xd,  0x11, 0x12, 0x14, 0x15, 0x17,
                           8,    0x1a, 0x1b, 0x1c, 0x1d, 0x20, 9,    0xa,  0x10, 0x13, 0xf,  0x16, 0x18, 0x19, 0x1e, 0x1f, 0xe};

u32 KindId(const u32 *object, u32 base) {
    const u32 kind = *object;
    if (kind == 0 || kind > 0x21)
        return 0x17f;
    return base + kOffsets[kind];
}

}  // namespace

extern "C" {

u32 __fastcall Kind_ToIdA(const u32 *object) { return KindId(object, 0xf7); }
u32 __fastcall Kind_ToIdB(const u32 *object) { return KindId(object, 0x118); }
u32 __fastcall Kind_ToIdC(const u32 *object) { return KindId(object, 0x139); }
u32 __fastcall Kind_ToIdD(const u32 *object) { return KindId(object, 0x15a); }

u32 __cdecl Index_ToCode485ec0(u32 x) {
    switch (x) {
    case 0:
    case 1:
        return 0;
    case 2:
        return 1;
    case 3:
        return 2;
    case 4:
        return 3;
    default:
        return 9;
    }
}

u32 __fastcall Byte2_ToIndex(const u8 *object) {
    switch (object[2]) {
    case 4:
        return 1;
    case 5:
        return 0;
    case 6:
        return 2;
    case 7:
        return 3;
    case 8:
        return 4;
    default:
        return 5;
    }
}

u32 __fastcall Mode19c_ToIndex(const u8 *object) {
    switch (*reinterpret_cast<const u32 *>(object + 0x19c)) {
    case 0:
        return 0;
    case 1:
        return 1;
    case 2:
    case 3:
        return 3;
    case 4:
    case 5:
        return 4;
    case 6:
        return 2;
    case 7:
        return 5;
    default:
        return 2;
    }
}

}  // extern "C"
