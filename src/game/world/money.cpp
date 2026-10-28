#include "money.h"

#include <cstdint>

#include "../core/filetime.h"
#include "../core/funnytime.h"
#include "../core/selectors.h"
#include "../core/vcall.h"
#include "../math/x87.h"
#include "analyser.h"
#include "object.h"
#include "thingmsg.h"
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

void __fastcall Money_MonthEnd(u8 *money) {
    Money_Credit(money, nullptr, at<s32>(money, 0x10));   // (inlined in the original, its debug print as well)
    at<u32>(money, 0x10) = 0;
    for (u32 i = 0; i < 8; ++i) {
        u8 *const loan = money + 0x20 + 0x20 * i;   // (+0 term, +4 rate, +8 taken, +0xc months paid)
        if (!at<u32>(loan, 8)) continue;
        const u32 rate = at<u32>(loan, 4);
        Money_Debit(money, nullptr, (s32)rate);   // (inlined in the original)
        ++at<u32>(loan, 0xc);
        const u32 term = at<u32>(loan, 0);
        if (term) at<u32>(money, 0x124) += at<u32>(loan, 4) - (at<u32>(loan, 4) * term - at<u32>(money, 0x18 + 0x20 * i)) / term;
        if (at<u32>(loan, 0xc) == at<u32>(loan, 0)) at<u32>(loan, 8) = at<u32>(loan, 0xc) = 0;
    }
    if (at<s32>(money, 0xc) >= 0) return;
    FileTime red;
    Money_TimeInRed(money, nullptr, &red);   // (inlined in the original)
    const long long months = (long long)((unsigned long long)red.high << 32 | red.low) / 0x1792f8648000LL;   // (30 days of 100 ns)
    if ((s32)months < 6) return;   // (the quotient's low dword, signed: sic)
    u8 msg[0xc];
    ThingMsg13_Ctor(msg, nullptr, 2);
    Object_PostMessage(msg);
}

namespace {
const BankAccountEnv kGameBankEnv = {reinterpret_cast<void(__fastcall *)(u8 *)>((uintptr_t)0x005168f0)};
const BankAccountEnv *g_bank = &kGameBankEnv;
}  // namespace

void BankAccount_SetEnv(const BankAccountEnv *env) { g_bank = env ? env : &kGameBankEnv; }

void __fastcall BankAccount_HandleMessage(u8 *money, void *, u8 *message) {
    switch (vcall<u32>(message, 0)) {
    case 0xc:
        Money_MonthEnd(money);
        return;
    case 0xd:
        at<u32>(money, 0x124) = 0;
        return;
    case 0x13:
        if (at<u32>(message, 8) == 2 && at<u32>(global<u8 *>(0x007cf83c), 0x1da738) != 4 && !Word7cf4f4_NonZero()) {
            g_bank->gameOver(global<u8 *>(0x007cf83c));
            return;
        }
        if (at<u32>(message, 8) == 0xa) {
            at<u32>(money, 0xc) = global<u32>(0x007851d8);
            at<u32>(money, 0x10) = 0;
        }
        return;
    }
}

u32 __fastcall Money_LoanOwed(const u8 *money, void *, u32 loan) {
    const u8 *entry = money + 0x20 + 0x20 * loan;
    return at<const u32>(entry, 8) ? (at<const u32>(entry, 0) - at<const u32>(entry, 0xc)) * at<const u32>(entry, 4) : 0;
}

u32 __fastcall Money_TotalOwed(const u8 *money) {
    u32 total = 0;
    for (u32 i = 0; i < 8; i++) total += Money_LoanOwed(money, nullptr, i);
    return total;
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

u32 *__fastcall Money_LoanOffer(u8 *money, void *, u32 *out, u32 index) {
    const u32 owed = Money_TotalOwed(money);
    u8 *const analyser = reinterpret_cast<u8 *>((uintptr_t)GameTable7cfb90_Get1da720_519510(global<u8 *>(0x0080239c)));
    const s32 score = (s32)ParkAnalyser_ParkScore(analyser, nullptr, 0x14);
    const u32 credit = (u32)(long long)(X87Reg((double)score) * global<float>(0x00700370)).v - owed;
    u32 *const entry = reinterpret_cast<u32 *>(money + 0x14 + 0x20 * index);
    entry[0] = (s32)entry[1] > (s32)credit ? 0 : 1;
    for (u32 i = 0; i < 8; i++) out[i] = entry[i];
    return out;
}

u32 __fastcall Money_AnyLoanOffer(u8 *money) {
    for (u32 i = 0; i < 8; ++i) {
        u32 offer[8];
        Money_LoanOffer(money, nullptr, offer, i);
        if (offer[5] == 0 && offer[0] != 0) return 1;
    }
    return 0;
}

}  // extern "C"
