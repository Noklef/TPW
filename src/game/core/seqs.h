// Small sequence helpers (rolling checksums, a character class, a range test, element counts and a block copy):
// reimplementation of 0x6a01a0, 0x6a7240, 0x6ae2b0, 0x6ae410, 0x6aed80, 0x6adf90 and 0x6af730. Named by what they do until the
// classes are known.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x6a01a0 / 0x6a7240 (stdcall; first, last, pops 8): a checksum over the 16-bit values / the signed bytes from first up to last:
// for each, the sum so far rotated left by 8 plus the value (sign extended for bytes). 0 for none.
u32 __stdcall Checksum_Words(const u16 *first, const u16 *last);
u32 __stdcall Checksum_SignedBytes(const s8 *first, const s8 *last);

// 0x6ae2b0 (cdecl; char): 1 for the 16-bit character 0x28 and 2 for 0x29, else 0.
u32 __cdecl Char_BracketClass(const u16 *c);

// 0x6ae410 (cdecl; first, last): whether the 16-bit value at the start of any 16-byte record from first up to last is above 0x5b0
// and below 0x5eb.
bool __cdecl Records16_AnyInRange5b0(const u8 *first, const u8 *last);

// 0x6aed80 / 0x6adf90 (thiscall): the number of 8-byte elements between the pointers at +4 and +8 / +0x88 and +0x8c; 0 when the
// first is 0.
s32 __fastcall Vector8_Count4(const u8 *object);
s32 __fastcall Vector8_Count88(const u8 *object);

// 0x6af730 (cdecl; first, last, dest): the 8-byte blocks from first up to last copied to dest; returns the end of what was
// written (undefined when there was nothing).
u8 *__cdecl Blocks8_Copy(const u8 *first, const u8 *last, u8 *dest);

// 0x6bfe10 (stdcall; text, length, pops 8): the last backslash below the length (not the first character) of the string *text
// points to: *text is advanced past it and its index is returned; else 0.
u32 __stdcall Path_AfterLastBackslash(const char **text, u32 length);

// 0x6c4010 (stdcall; dest, source, count, pops 12): count bytes copied forward (dwords, then the rest as bytes). Returns count.
u32 __stdcall Mem_CopyForward(void *dest, const void *source, u32 count);

// 0x6a7030 (stdcall; first1, last1, first2, last2, pops 16): a lexicographic comparison of two ranges of signed bytes: -1 when the
// first is less, 1 when it is greater (a longer first range being greater), else 0.
s32 __stdcall SignedBytes_Compare(const s8 *first1, const s8 *last1, const s8 *first2, const s8 *last2);

// 0x6afc90 (thiscall; text, count, pops 8): writes into a fixed buffer (the cursor at +4 and its end at +8, the full flag at
// +0x10): nothing when the cursor is already at or past the end; else count bytes of the text (a count of -1: up to the zero
// byte) are stored at the cursor, which moves on, until the cursor reaches the end, which sets the flag. Undefined return.
void __fastcall ByteBuf_Write(u8 *buffer, void *, const u8 *text, s32 count);

}  // extern "C"
