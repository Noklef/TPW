// The video library's coefficient code tables (the MPEG kind of run / level codes: unconfirmed). Built once from the static code
// lists at 0x7061d8 (0x5e records) and 0x7067b8 (0x80 records) of 16 bytes: the code's length, the run / level word, a dword unused
// here and the code left-aligned in 16 bits. Reimplementation of 0x676c80. The names are unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x676c80 (cdecl, no arguments; name unconfirmed): the decode tables: 0xfbc44c's first 16 dwords and 0xfbc84c's 0x80 the escape
// markers (0xf, 0x1f, 0x2f, 0x3f), then each code's entries ((word << 16 | word & 0xfc00) << 6 | the length to skip) over every index
// its prefix covers: the first list's long codes (any of bits 10-15 set) by their top 9 bits into 0xfbc44c, the short ones by their
// top 14 bits into 0xfbd04c (length - 6); the second list's (length + 8) by the top 9 bits into 0xfbcc4c (length - 9) unless bit 15 is
// set, else by the top 6 into 0xfbd04c (length - 6); 0xfbd44c's 0x20 dwords 1, then 0xfbd4cc's 32 the levels k << 22 | 6 for k 1-16
// and -16 to -1; and the built flag 0xfaac68 set.
void __cdecl DctVlc_BuildTables();

// 0x677c00 (cdecl, no arguments, returns the coefficient count; name unconfirmed): one block's coefficients from the bit reader
// (0xfbd64c the next 16-bit word, 0xfbd650 the bits left-aligned, 0xfbd654 their count; refilled a word at a time below 16): the DC
// from the top 8 bits (signed) * the quantiser 0xfbd54c into 0xfbc14c, the other 63 (0xfbc150 on) cleared, then codes until the end
// one: each by its top 9 bits in 0xfbc44c, or for the markers 0xf / 0x1f by the next 8 after 9 / 6 bits in 0xfbcc4c / 0xfbd04c, the
// escape 0x2f 6 bits then the run (6 bits) and level (10, signed) raw in 16, the end 0x3f 2 bits: the run skipped, the level (the
// entry's top 10 bits, signed) * the quantiser at the zigzag offset 0xfbc34c[k] stored at that offset. The listing keeps ECX and EDX
// (unused by its callers).
s32 __cdecl DctVlc_DecodeBlock();

// 0x6770b0 (fastcall (n); name unconfirmed): the bit reader's n bits dropped (the shift mod 32), and a word read in when fewer than
// 16 are left (compared unsigned: a negative count reads one too).
void __fastcall DctVlc_Skip(u32 n);

// 0x677110 (cdecl; name unconfirmed): the code in the top 6 bits by 0xfbd44c: its low byte the bits skipped (DctVlc_Skip), its top 10
// bits (signed) the answer.
s32 __cdecl DctVlc_ReadCode();

// 0x677140 (fastcall (out, pitch in dwords); name unconfirmed): an 8x8 block of dwords filled with the DC 0xfbc14c (read for each).
void __fastcall DctVlc_FillDc(s32 *out, s32 pitch);
}
