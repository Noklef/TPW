// The game's std::wstring: MSVC6 <xstring> basic_string<unsigned short>, whose out-of-line members the compiler emitted at 0x4f21d0, 0x4f39e0,
// 0x4f6090 and 0x4f6190 (its _Tidy is CountedBuf_Reset 0x4f23f0, its max_size 0x4f6080, its _Eos ImeWString_SetLength 0x47ba30; the rest is
// inlined). Reimplementation of those four; the narrow string is core/stdstring.
//
// As the narrow one: copy-on-write, the characters two bytes into a heap block (Mem_Alloc 0x67acc0) whose second byte (just before the
// characters) counts the other owners (0: one owner, 0xff: frozen). A string is { allocator (empty), ptr, len, res }; res is at least 31 once
// allocated, and the block is (res + 2) * 2 bytes. A null ptr reads as _Nullstr (0x700624).
//
// Library bugs kept (sic), as the narrow one's:
//  - _Copy(n) copies all len old characters into a block sized for n | 31.
//  - _Copy's catch(...) retry with the exact size is unreachable (the game's operator new returns null), so it is left out.
#pragma once

#include "../../../re/types.h"

struct WStdString {
    u32 allocator;  // std::allocator<unsigned short>: empty, never read
    u16 *ptr;
    u32 len;
    u32 res;
};
static_assert(sizeof(WStdString) == 16, "MSVC6 basic_string is 16 bytes");

// All __thiscall (self in ECX), written as __fastcall with an unused EDX.
extern "C" {

// 0x4f21d0: assign(const wstring &x, pos, n): x's characters from pos (past x's length: _Xran), at most n. Shares x's block (its count
// raised) when n is all of x, not 0, and x's count is below 0xfe; assigning part of itself erases the tail (from pos + n, at most the wide
// npos at 0x700628) and then the head in place. Returns s.
WStdString *__fastcall WStdString_AssignSub(WStdString *s, void *edx, const WStdString *x, u32 pos, u32 n);

// 0x4f39e0: _Grow(n, trim): room for n characters in a block s owns alone (above 0x7ffffffd: _Xlen). A shared block: with n 0 one count
// less and s emptied, else _Copy(n). Else n 0: emptied (trim: the block freed) or ended at 0 (an existing block). Else with trim, a
// block above 31 or below n is freed first and a new one made (_Copy(n)); without trim, only a block below n is replaced (_Copy(n)).
// Returns whether there are characters to write (0 when n is 0).
u8 __fastcall WStdString_Grow(WStdString *s, void *edx, u32 n, u8 trim);

// 0x4f6090: _Copy(n): a new block of n | 31 characters (n above 0x7ffffffd: n; the count asked for is (res + 2) * 2, a negative res + 2
// counting as 0) gets the len old characters; the old block is released (one count less, or freed); the count byte 0, res set and the
// length kept, ended there.
void __fastcall WStdString_Copy(WStdString *s, void *edx, u32 n);

// 0x4f6190: _Split(): a shared s gets its own copy: the old block loses a count, s is emptied and assigned the old characters up to their
// first 0 (wcslen).
void __fastcall WStdString_Split(WStdString *s, void *edx);
}
