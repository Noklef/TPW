// The park's bank account (thing model 16: balance, admission fee, eight loans): reimplementation of its constructor 0x4cf7c0 and serializer 0x4cf920. The fields'
// meanings are the symbols' (high confidence for the model and the loans, not for each field).
#pragma once

#include "../../../re/types.h"

struct Stream;

struct BankAccountCtorEnv {
    double(__cdecl *pow)(double x, double y);  // the game's _CIpow 0x67c5d0
};
void BankAccountCtor_SetEnv(const BankAccountCtorEnv *env);

extern "C" {

// 0x4cf7c0 (thiscall; the name unconfirmed): the thing constructor (Thing_Construct, cell 1, kind 0x10); +0xc from [0x7851d8], +0x118 from [0x7851dc], +0x114 = 1, the other head fields 0; the eight loans (from +0x14, 0x20 apart) from the
// table 0x7851e0 (0x10 each): +0 = amount <= 10000, +4 amount, +8 rate, +0xc term, +0x10 the amount * (rate * 0.01 + 1) ^ (term / 12 * 0.5) / term (truncated), +0x14 and +0x18 = 0, +0x1c the fourth dword. The admission fee's print is left
// out; the account is added to the message centre's slots 0xc, 0x13 and 0xd. Returns account.
u8 *__fastcall BankAccount_Ctor(u8 *account);

// 0x4cf920 (thiscall; stream, save, version, unused; pops 16): 1 when Thing_Serialize is not 0; 0 for versions other than 0 and 1. Saving:
// the dwords +0x118, +0xc, +0x10, +0x114, +0x11c, +0x120, +0x124, then the eight loans' eight dwords each (from +0x14, 0x20 apart), each
// through Save_Field (the names 0x759df4 ... 0x759c9c). Loading: +0x118, +0xc and +0x10 the same way, the rest through Save_ReadU32
// (announced). 1 at the first field that fails (the debug print left out), else 0.
u32 __fastcall BankAccount_Serialize(u8 *account, void *, Stream *stream, u32 save, s32 version, u32 unused);
}
