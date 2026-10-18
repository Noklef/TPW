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

// 0x6af960 (thiscall; key, pops 4): a binary search for the key (unsigned) among the first dwords of records of 12 bytes from the
// dword at +8 up to the one at +0xc (inclusive): the record, or 0.
u8 *__fastcall Table12_Find(const u8 *table, void *, u32 key);

// 0x5b3210 (cdecl; dest, src): *(u16 *)dest = *(u16 *)src, unless dest is 0.
void __cdecl Word_CopyTo(u16 *dest, const u16 *src);

// 0x6322e0 (cdecl; first, last, dest): the dwords from first up to last copied to dest, forwards;
// returns the end of what was written (dest when none).
u32 *__cdecl Dwords_Copy(const u32 *first, const u32 *last, u32 *dest);

// 0x50f240 (cdecl): the 8-byte entries from the top of the stack at 0x7cf07c (a pointer) down to
// its base at 0x7cf078 zeroed, the top moving down as it goes; nothing when it is at or below
// the base.
void __cdecl EntryStack_Clear();

// 0x5633e0 (stdcall; source, unused, unused, object, pops 16): when the count at +0x788 is below 4 the next 16-byte slot from +0x448 gets the four
// dwords from source (zeros for 0) and the count rises. Returns 1.
u32 __stdcall Quad_Push(const u32 *source, u32, u32, u8 *object);

// 0x56ee90 (thiscall; four bytes b0..b3 as one dword, pops 4): 0 unless the signed dword at +0x128 <= b0, the signed dword at +0x12c >= b1 and b1 <= b0;
// then 1 when for some i from b1 to b0 the signed byte at +0x130 + i is at most b2 and the signed byte at +0x1b0 + i is at least b3 (b2 and b3 as bytes).
u32 __fastcall Bands_AnyMatch(const u8 *object, void *, u32 packed);

// 0x56f4f0 (stdcall; four bytes, flags word, pops 8): nothing unless the word has a bit of 0x78; bit 0x40 swaps bytes 1 and 3; then bit 8 rotates the bytes
// one place to the right (the last comes first), otherwise bit 0x10 swaps the halves, otherwise bit 0x20 rotates one place to the left.
void __stdcall Bytes4_Permute(u8 *bytes, const u16 *flags);

// 0x598770 (cdecl; pointer, count): count dwords from the pointer are set to 0. Returns 0.
u32 __cdecl Dwords_Zero(u32 *to, u32 count);

// 0x56f2f0 (thiscall on the bands object, unused but passed on; values, lo, hi; pops 0xc; name unconfirmed; 0x56f380 asks it for
// 0..count-1): a quicksort of the dwords values[lo..hi], ascending (unsigned). The pivot is values[(lo + hi) >> 1] (32-bit sum, signed
// shift); i runs up from lo while at most hi and below the pivot; then, only when i is past lo (sic), j runs down from hi while above it,
// unbounded; i <= j swaps the two and moves both on, again while i <= j. The left part (lo, j) recurses when lo < j and the right
// (i, hi) loops when i < hi (all signed).
void __fastcall Bands_SortValues(const u8 *object, void *, u32 *values, s32 lo, s32 hi);
}  // extern "C"
