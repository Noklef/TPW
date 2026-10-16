// Searches of the owner's item list: reimplementation of 0x5bc9c0, 0x5bcb60, 0x5bd270, 0x5bd670,
// 0x5bd690 and 0x5bd9d0 (the object of Online_User5c's callers).
//
// The owner's +0x2c points at a list whose +4 is the first node; a node is {item, next}. Every
// search walks the whole list and keeps the last match (sic), 0 when nothing matches.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x5bc9c0 (thiscall; key): the item whose +8 is the key.
u8 *__fastcall Owner2c_FindByKey(u8 *owner, void *, u32 key);

// 0x5bcb60 (thiscall; index): the index'th item (0 first).
u8 *__fastcall Owner2c_At(u8 *owner, void *, s32 index);

// 0x5bd270 (thiscall; key): the +0x14 of the item whose +8 is the key, 0 when none.
u32 __fastcall Owner2c_ValueByKey(u8 *owner, void *, u32 key);

// 0x5bd670 / 0x5bd690 (thiscall): the item whose +0xc is 1 / 0x64.
u8 *__fastcall Owner2c_FindKind1(u8 *owner);
u8 *__fastcall Owner2c_FindKind100(u8 *owner);

// 0x5bd9d0 (thiscall; a, b): the item whose +0x44 is a and +0x64 is b.
u8 *__fastcall Owner2c_FindByPair(u8 *owner, void *, u32 a, u32 b);
}
