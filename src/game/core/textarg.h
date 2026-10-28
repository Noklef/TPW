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

// 0x6add40 (the number argument's method, thiscall (writer, state), pops 8; name invented): +0xc formatted with L"%d" (swprintf; the
// minus flag +0x10 not looked at, sic) into 0x12 cells on the stack, written through the writer's slot 5 (text, length). Returns 0.
s32 __fastcall TextArgInt_Write(u8 *arg, void *, u8 *writer, void *state);

// 0x6add70 (the string argument's method, thiscall (writer, state), pops 8; name invented): the string (+0xc) written whole through
// the writer's slot 5 (string, -1). Returns 0.
s32 __fastcall TextArgString_Write(u8 *arg, void *, u8 *writer, void *state);
}

// The text formatter 0x6acd60 (cdecl: a text table, the buffer, its size in characters, the text's id, then the arguments for its %n
// places; library code under SEH, not ported) and its narrow twin 0x6acca0 (the same into a byte buffer): ports call them through
// TextFormat(), which tools/fncmp's driver swaps for log stubs.
struct TextFormatEnv {
    u32(__cdecl *format)(void *table, u16 *to, u32 size, u32 id, ...);
    u32(__cdecl *formatNarrow)(void *table, u8 *to, u32 size, u32 id, ...);
};
const TextFormatEnv &TextFormat();
void TextFormat_SetEnv(const TextFormatEnv *env);  // nullptr: the game's
