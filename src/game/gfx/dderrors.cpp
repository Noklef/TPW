#include "dderrors.h"

#include <cstdint>

extern "C" {

const char *__fastcall DdErrors_Name(u32 code) {
    const u32 *const table = reinterpret_cast<const u32 *>((uintptr_t)0x00705ee8);
    for (u32 k = 0; k < (0x007061c8 - 0x00705ee8) / 8; ++k)
        if (table[2 * k] == code) return reinterpret_cast<const char *>((uintptr_t)table[2 * k + 1]);
    return reinterpret_cast<const char *>((uintptr_t)0x0077e528);
}
}
