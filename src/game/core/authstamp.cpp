#include "authstamp.h"

#include <cstdint>

extern "C" {

u8 *__cdecl AuthStamp_Text() {
    const u8 *const phrase = reinterpret_cast<const u8 *>((uintptr_t)0x00763f3c);
    const u8 *const key = reinterpret_cast<const u8 *>((uintptr_t)0x00774b78);
    const u8 *const plain = reinterpret_cast<const u8 *>((uintptr_t)0x00774bd8);
    u8 *const out = reinterpret_cast<u8 *>((uintptr_t)0x00f827f8);
    u32 i = 0;
    while (phrase[1 + i] == plain[i] && phrase[1 + i])
        ++i;
    if (phrase[1 + i] == plain[i])
        return reinterpret_cast<u8 *>((uintptr_t)0x00774bc0);
    u8 p = phrase[0], k = key[0];
    for (u32 j = 0; p != k; ++j) {
        out[j] = (u8)(p - k);
        k = key[1 + j];
        p = phrase[1 + j];
    }
    return out;
}

}  // extern "C"
