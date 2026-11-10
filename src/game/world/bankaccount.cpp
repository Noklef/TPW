#include "bankaccount.h"

#include <cstdint>

#include "../../hook/regthunks.h"
#include "../core/savegame.h"
#include "../math/x87.h"
#include "messagecentre.h"
#include "things.h"

namespace {

__declspec(naked) double __cdecl GamePow(double, double) {
    __asm {
        fld qword ptr [esp + 4]
        fld qword ptr [esp + 12]
        call RegThunk_CRT_CIpow
        ret
    }
}

const BankAccountCtorEnv kGameEnv = {&GamePow};
const BankAccountCtorEnv *g_env = &kGameEnv;

u32 &dword(u8 *object, u32 at) { return *reinterpret_cast<u32 *>(object + at); }

// The head's fields as saved (Save_Field each); a load reads the first three the same way and the rest through Save_ReadU32.
const SaveSpec kHead[] = {{0x118, 4, 0x00759df4}, {0xc, 4, 0x00759de8}, {0x10, 4, 0x00759dd8}, {0x114, 4, 0x00759dc4},
                          {0x11c, 4, 0x00759db4}, {0x120, 4, 0x00759da4}, {0x124, 4, 0x00759d94}};
// A loan's eight dwords (the first loan's offsets; the others 0x20 on).
const SaveSpec kLoan[] = {{0x14, 4, 0x00759d78}, {0x18, 4, 0x00759d58}, {0x1c, 4, 0x00759d3c}, {0x20, 4, 0x00759d14},
                          {0x24, 4, 0x00759cf4}, {0x28, 4, 0x00759cd8}, {0x2c, 4, 0x00759cbc}, {0x30, 4, 0x00759c9c}};

bool ReadU32s(Stream *stream, u8 *object, const SaveSpec *spec, u32 count) {
    for (u32 i = 0; i < count; ++i)
        if (Save_ReadU32(stream, object + spec[i].at, reinterpret_cast<const char *>(spec[i].name), 0) != 0) return false;
    return true;
}

}  // namespace

void BankAccountCtor_SetEnv(const BankAccountCtorEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

u8 *__fastcall BankAccount_Ctor(u8 *account) {
    Thing_Construct(account, nullptr, 1, 0x10);
    const u32 *const head = reinterpret_cast<const u32 *>((uintptr_t)0x007851d8);
    dword(account, 0xc) = head[0];
    dword(account, 0x10) = 0;
    dword(account, 0x114) = 1;
    dword(account, 0x118) = head[1];
    dword(account, 0x11c) = 0;
    dword(account, 0x120) = 0;
    dword(account, 0x124) = 0;
    for (u32 k = 0; k < 8; ++k) {
        u8 *const loan = account + 0x14 + k * 0x20;
        const u32 *const row = head + 2 + k * 4;
        const u32 amount = row[0];
        dword(loan, 0) = amount <= 10000 ? 1 : 0;
        dword(loan, 4) = amount;
        dword(loan, 8) = row[1];
        dword(loan, 0xc) = row[2];
        dword(loan, 0x14) = 0;
        dword(loan, 0x18) = 0;
        dword(loan, 0x1c) = row[3];
        const double base = (X87Reg(X87_MulUIntDouble(row[1], 0.01)) - X87Reg(-1.0)).v;
        const double power = (X87Reg(X87_MulUIntDouble(row[2], 0.08333333333333333)) * X87Reg(0.5)).v;
        const double growth = g_env->pow(base, power);
        dword(loan, 0x10) = static_cast<u32>(static_cast<__int64>((X87Reg(X87_MulUIntDouble(amount, growth)) / X87Reg(static_cast<double>(row[2]))).v));
    }
    MessageCentre_AddThing(MessageCentre_Instance(), nullptr, account, 0xc);
    MessageCentre_AddThing(MessageCentre_Instance(), nullptr, account, 0x13);
    MessageCentre_AddThing(MessageCentre_Instance(), nullptr, account, 0xd);
    return account;
}

u32 __fastcall BankAccount_Serialize(u8 *account, void *, Stream *stream, u32 save, s32 version, u32 unused) {
    if (Thing_Serialize(account, nullptr, stream, save, version, unused) != 0) return 1;
    if (version < 0 || version > 1) return 0;
    if (save != 0) {
        if (!Save_Fields(stream, save, account, kHead, 7)) return 1;
        for (u32 loan = 0; loan < 8; ++loan)
            if (!Save_Fields(stream, save, account + loan * 0x20, kLoan, 8)) return 1;
        return 0;
    }
    if (!Save_Fields(stream, 0, account, kHead, 3) || !ReadU32s(stream, account, kHead + 3, 4)) return 1;
    for (u32 loan = 0; loan < 8; ++loan)
        if (!ReadU32s(stream, account + loan * 0x20, kLoan, 8)) return 1;
    return 0;
}
}
