#include "tiny_m.h"

#include <cstdint>


namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

}  // namespace

extern "C" {

u32 __fastcall GameTable7cfb90_Get1da718_519450(u8 *o) {
    return global<u32>(0x007cfb90 + 20 * at<u16>(o, 0x1da718));
}

u32 __fastcall GameTable7cfb90_Get1da71a_519490(u8 *o) {
    return global<u32>(0x007cfb90 + 20 * at<u16>(o, 0x1da71a));
}

u32 __fastcall GameTable7cfb90_Get1da71c_5194d0(u8 *o) {
    return global<u32>(0x007cfb90 + 20 * at<u16>(o, 0x1da71c));
}

u32 __fastcall GameTable7cfb90_Get1da720_519510(u8 *o) {
    return global<u32>(0x007cfb90 + 20 * at<u16>(o, 0x1da720));
}

u32 __fastcall GameTable7cfb90_Get1da722_519550(u8 *o) {
    return global<u32>(0x007cfb90 + 20 * at<u16>(o, 0x1da722));
}

u32 __fastcall GameTable7cfb90_Get1da724_519590(u8 *o) {
    return global<u32>(0x007cfb90 + 20 * at<u16>(o, 0x1da724));
}

u32 __fastcall GameTable7cfb90_Get1da726_5195d0(u8 *o) {
    return global<u32>(0x007cfb90 + 20 * at<u16>(o, 0x1da726));
}

u32 __fastcall GameTable7cfb90_Get1da736_519610(u8 *o) {
    return global<u32>(0x007cfb90 + 20 * at<u16>(o, 0x1da736));
}

}
