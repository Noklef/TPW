#include "money.h"

#include <cstdint>

#include "../core/tiny_m.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

}  // namespace

extern "C" {

void __fastcall Money_Credit(u8 *money, void *, s32 amount) {
    at<u32>(money, 0xc) += (u32)amount;
    u8 *thing = reinterpret_cast<u8 *>(GameTable7cfb90_Get1da720_519510(global<u8 *>(0x007cf83c)));
    at<u32>(thing, 0x1fc90) += (u32)amount;
    at<u32>(money, 0x124) += (u32)amount;
}

void __fastcall Money_Debit(u8 *money, void *, s32 amount) {
    if (!at<u32>(money, 0x114))
        return;
    const u32 balance = at<u32>(money, 0xc) - (u32)amount;
    at<u32>(money, 0xc) = balance;
    if ((s32)balance < 0 && at<s32>(money, 0x11c) >= 0)
        at<u32>(money, 0x120) = at<u32>(global<u8 *>(0x0080239c), 0x1da70c);
    at<u32>(money, 0x11c) = balance;
    u8 *thing = reinterpret_cast<u8 *>(GameTable7cfb90_Get1da720_519510(global<u8 *>(0x007cf83c)));
    at<u32>(thing, 0x1f5a0) += (u32)amount;
    at<u32>(money, 0x124) -= (u32)amount;
}

}  // extern "C"
