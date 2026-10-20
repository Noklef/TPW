// The park's bank account (thing model 16: balance, admission fee, eight loans): reimplementation of its serializer 0x4cf920. The fields'
// meanings are the symbols' (high confidence for the model and the loans, not for each field).
#pragma once

#include "../../../re/types.h"

struct Stream;

extern "C" {

// 0x4cf920 (thiscall; stream, save, version, unused; pops 16): 1 when Thing_Serialize is not 0; 0 for versions other than 0 and 1. Saving:
// the dwords +0x118, +0xc, +0x10, +0x114, +0x11c, +0x120, +0x124, then the eight loans' eight dwords each (from +0x14, 0x20 apart), each
// through Save_Field (the names 0x759df4 ... 0x759c9c). Loading: +0x118, +0xc and +0x10 the same way, the rest through Save_ReadU32
// (announced). 1 at the first field that fails (the debug print left out), else 0.
u32 __fastcall BankAccount_Serialize(u8 *account, void *, Stream *stream, u32 save, s32 version, u32 unused);
}
