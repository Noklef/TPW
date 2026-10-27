// The game's circular buffers (CircBufU8 / U16 / U32, serialized at 0x4cea00, 0x4cdb80 and
// 0x4cdf00 as mCurrentEntry, mNumEntries, mWrappedAround, mTemp, mData): reimplementation of
// 0x4cdb60, 0x4ce9e0, 0x4ce290, 0x495c20, 0x495cf0 and 0x51ade0-0x51ae90, each for one size (thiscall). mTemp is the first
// value; the entries follow it, then the cursor (-1 when empty), the size and the wrapped flag.
#pragma once

#include "../../../re/types.h"
#include "../files/file.h"

extern "C" {

// 0x4801d0 / 0x480380 (cdecl; buffer, out, kind): the greatest / least of the newest samples of a CircBufU32 of
// 144 (values from +4, cursor +0x244, entries +0x248, wrapped flag +0x24c) within a window of 12 samples
// (kind 0), 36 (1), 144 (2) or 0 (others), newest first, counted from the cursor back (wrapping if the buffer
// has wrapped). Only samples above 0 (below 0 for the least) are considered, the first of equals; the scan
// ends at the window, the entry count or a sample that is not there. The value at the chosen index is then
// read again (the window itself when there is none: 0x4801d0 even when nothing was chosen, reading the sample
// beside the cursor; 0x480380 not at all then) and *out is raised / lowered to it (signed).
void __cdecl CircBufU32_144_MaxInWindow(const u8 *buffer, s32 *out, u32 kind);
void __cdecl CircBufU32_144_MinInWindow(const u8 *buffer, s32 *out, u32 kind);

// 0x4802a0 (cdecl; buffer, out, kind; name unconfirmed): CircBufU32_144_MaxInWindow for a buffer of 144 floats: the greatest sample
// above 0.0 (not a NaN) is read again as there, or when it cannot be (none chosen past the count, or not there) the greatest
// itself (0.0 for none); *out is raised to its __ftol.
void __cdecl CircBufF32_144_MaxInWindow(const u8 *buffer, s32 *out, u32 kind);

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

// 0x495d40 (thiscall; name unconfirmed): the entries it holds: the size (+0x80) once wrapped (the byte +0x84), else the cursor (+0x7c) + 1.
u32 __fastcall CircBufU32_30_Count(const u8 *buf);

// 0x495c20 (&sum, count; pops 8; name unconfirmed): the CHistory sum: *sum (a double) = 0, then each of the newest count entries (at most the
// size once wrapped, else the cursor + 1; signed) that CircBufU32_30_Get finds (inlined) added to it on the x87 (fild of the unsigned value,
// fadd of the sum: rounded to 24 bits). (The debug print of whether count is within the size is left out.)
void __fastcall CircBufU32_30_Sum(const u8 *buf, void *, double *sum, s32 count);

// 0x4cef30 (&sum, count; pops 8; name unconfirmed): CircBufU32_30_Sum for a CircBufU32 of 50 (cursor +0xcc, size +0xd0, wrapped +0xd4), its
// look back inlined, the values loaded signed (fild of the dword). (The debug print of whether count is within the size is left out.) Returns
// the count as cut to the entries held (the park analyser's 0x4c79fc divides by it).
s32 __fastcall CircBufU32_50_Sum(const u8 *buf, void *, double *sum, s32 count);

// 0x4e2c10 (thiscall; stream, save, version, unused): a CircBufU32 of 30 saved or loaded as a serializer does (Save_Field; the entries'
// load through Save_ReadU32): mCurrentEntry (+0x7c), mNumEntries (+0x80), mWrappedAround (+0x84), mTemp (+0); then mData[i] (from +4)
// while i < mNumEntries (signed, read again each time: a loaded count over 30 runs past the buffer, sic).
u32 __fastcall CircBufU32_30_Serialize(u8 *buf, void *, Stream *stream, u32 save, s32 version, u32 unused);

// 0x4cdf00 (likewise): the same for a CircBufU32 of 144 (+0x244, +0x248, +0x24c; the same names).
u32 __fastcall CircBufU32_144_Serialize(u8 *buf, void *, Stream *stream, u32 save, s32 version, u32 unused);

// 0x4ce2d0 / 0x4ce660 (likewise; names unconfirmed): the same for a CircBufU32 of 50 (+0xcc, +0xd0, +0xd4) and of 5 (+0x18, +0x1c, +0x20).
u32 __fastcall CircBufU32_50_Serialize(u8 *buf, void *, Stream *stream, u32 save, s32 version, u32 unused);
u32 __fastcall CircBufU32_5_Serialize(u8 *buf, void *, Stream *stream, u32 save, s32 version, u32 unused);

// 0x4cdb80 / 0x4cea00 (likewise): the same for a CircBufU16 (its mTemp a word, the entries words from +2: +8, +0xc, +0x10; the entries'
// load through Save_ReadU16) and a CircBufU8 of 50 (bytes from +1: +0x34, +0x38, +0x3c; Save_ReadU8).
u32 __fastcall CircBufU16_Serialize(u8 *buf, void *, Stream *stream, u32 save, s32 version, u32 unused);
u32 __fastcall CircBufU8_Serialize(u8 *buf, void *, Stream *stream, u32 save, s32 version, u32 unused);
}
