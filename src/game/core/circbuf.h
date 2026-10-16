// The game's circular buffers (CircBufU8 / U16 / U32, serialized at 0x4cea00, 0x4cdb80 and
// 0x4cdf00 as mCurrentEntry, mNumEntries, mWrappedAround, mTemp, mData): reimplementation of
// 0x4cdb60, 0x4ce9e0, 0x4ce290, 0x495cf0 and 0x51ade0-0x51ae90, each for one size (thiscall). mTemp is the first
// value; the entries follow it, then the cursor (-1 when empty), the size and the wrapped flag.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x4cdb60: a CircBufU16 of 3 (cursor +8, size +0xc, wrapped +0x10): u16 +0 = 0, cursor -1,
// size 3, not wrapped.
u8 *__fastcall CircBufU16_3_Ctor(u8 *buf);

// 0x4ce9e0: a CircBufU8 of 50 (cursor +0x34, size +0x38, wrapped +0x3c): byte +0 = 0, cursor
// -1, size 50, not wrapped.
u8 *__fastcall CircBufU8_50_Ctor(u8 *buf);

// 0x51ade0, 0x51ae30, 0x51ae60, 0x51ae90: CircBufU32s of 30, 144, 50 and 5 (the values from +4,
// then the cursor, size and wrapped flag): mTemp 0, cursor -1, the size, not wrapped.
u8 *__fastcall CircBufU32_30_Ctor(u8 *buf);
u8 *__fastcall CircBufU32_144_Ctor(u8 *buf);
u8 *__fastcall CircBufU32_50_Ctor(u8 *buf);
u8 *__fastcall CircBufU32_5_Ctor(u8 *buf);

// 0x4ce290 (value): a CircBufU32 of 144 (values from +4, cursor +0x244, size +0x248, wrapped
// +0x24c): the cursor on (back to 0 at the size, signed, setting wrapped), the value there.
void __fastcall CircBufU32_144_Push(u8 *buf, void *, u32 value);

// 0x495cf0 (&value, back): a CircBufU32 of 30 (values from +4, cursor +0x7c, size +0x80,
// wrapped +0x84): the value back entries before the cursor (signed compares; past the start
// only once wrapped) to *out, and 1; 0 (out untouched) when there is none. Returns in AL only.
u8 __fastcall CircBufU32_30_Get(const u8 *buf, void *, u32 *out, s32 back);
}
