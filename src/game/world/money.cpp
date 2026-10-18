#include "money.h"

#include <cstdint>

#include "../core/filetime.h"
#include "../core/funnytime.h"
#include "world.h"

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

u32 __fastcall Money_Total(const u8 *money) { return at<u32>(money, 0x124); }

FileTime *__fastcall Money_TimeInRed(const u8 *money, void *, FileTime *out) {
    if (at<s32>(money, 0x11c) >= 0) {
        out->low = out->high = 0;
        return out;
    }
    // (the original's debug check that the time entered is not after the world's is left out)
    return FunnyTime_SpanSince(global<const u8 *>(0x0080239c) + 0x2a0, nullptr, out, at<u32>(money, 0x120));
}

s32 __fastcall Obj_SmallestEntry(const u8 *object) {
    s32 best = -1;
    u32 smallest = 0;
    bool found = false;
    for (u32 i = 0; i < 8; i++) {
        const u8 *entry = object + 0x20 + 0x20 * i;
        const u32 size = at<const u32>(entry, 8) ? (at<const u32>(entry, 0) - at<const u32>(entry, 0xc)) * at<const u32>(entry, 4) : 0;
        if (size > 0 && (size < smallest || !found)) {
            best = (s32)i;
            smallest = size;
            found = true;
        }
    }
    return best;
}

}  // extern "C"
