#include "tiny_h.h"

#include <cstdint>

#include "tiny_g.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

}  // namespace

extern "C" {

u8 * __cdecl Wrap_Wrap_Init_6b4a60_6b0710_6b0140() {
    return Wrap_Init_6b4a60_6b0710(reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(0x00fb1efc)));
}

u8 * __cdecl Wrap_Wrap_Init_6b4a60_6b1de0_6b0180() {
    return Wrap_Init_6b4a60_6b1de0(reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(0x00fb1ef0)));
}

u8 * __cdecl Wrap_Wrap_Init_6b4a60_6b2a10_6b01c0() {
    return Wrap_Init_6b4a60_6b2a10(reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(0x00fb1ef4)));
}

u8 * __cdecl Wrap_Wrap_Init_6b4a60_6b3c00_6b0200() {
    return Wrap_Init_6b4a60_6b3c00(reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(0x00fb1ef8)));
}

}
