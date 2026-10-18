// The user slots object at [0xf81ebc] (0x110 bytes, made by 0x5c7420; subject unconfirmed): four records of 24 bytes from +0, each a
// wide String (+4 its length) and a dword at +0x14, then +0x60 = -1, a 0x5cadf0 object at +0x64 and a list head at +0x100. Its
// filler (0x5c7590) walks the save folder's "users" subfolders (DirEntry_IsRealDir); the dialog code at 0x4a6xxx asks it for the
// slots in use. Reimplementation of the record accessors 0x5c7bb0, 0x5c7d00 and 0x5c7d20.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x5c7bb0 (thiscall): how many of the four 24-byte records have a non-zero dword at +4.
u32 __fastcall Records24_CountUsed(const u8 *records);

// 0x5c7d00 (thiscall; i, pops 4): the address of record i of 24 bytes when its dword at +4 is not 0, else 0.
u8 *__fastcall Records24_IfUsed(u8 *records, void *, u32 i);

// 0x5c7d20 (thiscall; i, pops 4): the dword at +0x14 of record i of 24 bytes.
u32 __fastcall Records24_Field14(const u8 *records, void *, u32 i);
}
