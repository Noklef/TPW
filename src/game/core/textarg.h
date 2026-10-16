// The text formatter's arguments (built for 0x6acd60, which puts each into its %n place):
// reimplementation of the out-of-line constructors 0x4a77a0, 0x4ea220 and 0x50aef0 (thiscall);
// the game builds most of them inline. Each is a vtable (+0; its one method writes the argument
// to a sink), the tag 'MRAV' (+4, 0x5641524d) and the argument's number (+8).
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x4a77a0 (n): the abstract base (vtable 0x6ffb34, its method _purecall, 0x67b0c0).
u8 *__fastcall TextArg_Ctor(u8 *arg, void *, u32 n);

// 0x4ea220 (n, string): a string argument (vtable 0x6ff174; written whole).
u8 *__fastcall TextArgString_Ctor(u8 *arg, void *, u32 n, u32 string);

// 0x50aef0 (n, value, isSigned): a number argument (vtable 0x6ff170; written with L"%d" from
// +0xc). A signed negative value keeps its magnitude (+0xc) and a minus flag (+0x10, 1); any
// other keeps the value and 0.
u8 *__fastcall TextArgInt_Ctor(u8 *arg, void *, u32 n, s32 value, u32 isSigned);
}
