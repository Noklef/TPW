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

u32 __fastcall Money_TakeLoan(u8 *money, void *, u32 index) {
    u8 *loan = money + index * 0x20;
    if (at<u32>(loan, 0x28) != 0)
        return 0;
    const u32 amount = at<u32>(loan, 0x18);
    at<u32>(money, 0xc) += amount;
    u8 *thing = reinterpret_cast<u8 *>(GameTable7cfb90_Get1da720_519510(global<u8 *>(0x007cf83c)));
    at<u32>(thing, 0x1fc90) += amount;
    u32 total = at<u32>(money, 0x124) + amount;
    at<u32>(money, 0x124) = total - at<u32>(loan, 0x18);
    at<u32>(loan, 0x28) = 1;
    return 1;
}

u32 __fastcall Money_PayOffLoan(u8 *money, void *, u32 index) {
    u8 *loan = money + index * 0x20;
    if (at<u32>(loan, 0x28) == 0)
        return 0;
    u8 *next = money + (index + 1) * 0x20;
    const u32 owed = (at<u32>(next, 0) - at<u32>(loan, 0x2c)) * at<u32>(loan, 0x24);
    if (at<u32>(money, 0xc) < owed)
        return 0;
    if (at<u32>(money, 0x114) != 0) {
        const u32 balance = at<u32>(money, 0xc) - owed;
        at<u32>(money, 0xc) = balance;
        if ((s32)balance < 0 && at<s32>(money, 0x11c) >= 0)
            at<u32>(money, 0x120) = at<u32>(global<u8 *>(0x0080239c), 0x1da70c);
        at<u32>(money, 0x11c) = balance;
        u8 *thing = reinterpret_cast<u8 *>(GameTable7cfb90_Get1da720_519510(global<u8 *>(0x007cf83c)));
        at<u32>(thing, 0x1f5a0) += owed;
        at<u32>(money, 0x124) -= owed;
    }
    at<u32>(loan, 0x28) = 0;
    at<u32>(loan, 0x2c) = 0;
    const u32 term = at<u32>(next, 0);
    if (term != 0) {
        const u32 monthly = (term * at<u32>(loan, 0x24) - at<u32>(loan, 0x18)) / term;
        at<u32>(money, 0x124) += owed - monthly * (term - at<u32>(loan, 0x2c));
    }
    return 1;
}

}  // extern "C"
