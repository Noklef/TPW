// The game's std::wstring: MSVC6 <xstring> basic_string<wchar_t> (the IME's strings), whose
// out-of-line members the compiler emitted at 0x405280 and 0x47b750-0x47bcd5. The exe holds a
// second copy of the same template with its own _Nullstr (0x700624) and npos (0x700628), whose
// members are at 0x4f21d0, 0x4f39e0, 0x4f6090 and 0x4f6190 (the WStdString_* functions below:
// the same layout and logic, kept as their own functions because the originals are separate code;
// that copy calls std::_Xlen / std::_Xran directly). Reimplementation of both copies' members;
// the wide string's others are already ported elsewhere: _Tidy (the counted
// buffer reset, CountedBuf_Reset 0x4f23f0), _Eos (ImeWString_SetLength 0x47ba30), max_size
// (Const7ffffffd_4f6080) and the traits' move (WTraits_Move 0x479150).
//
// Copy-on-write as the narrow string (stdstring.h): the characters sit two bytes into a heap
// block (Mem_Alloc 0x67acc0) whose second byte (the byte before the characters) counts the other
// owners (0: one owner, 0xff: frozen, never shared). A string is { allocator (empty), ptr, len,
// res }; res is the capacity, at least 31 once allocated (_MIN_SIZE), and the block is
// (res + 2) * 2 bytes. A null ptr reads as _Nullstr (0x6fd6c4).
//
// Library bugs kept (sic), as the narrow string's:
//  - _Copy(n) copies all len old characters into a block sized for n | 31.
//  - _Copy's catch(...) retry with the exact size is unreachable (the game's operator new returns
//    null rather than throwing), so it is left out.
// The throws (std::_Xlen 0x6918f0, std::_Xran 0x691ba0) go through the narrow strings' env.
#pragma once

#include "../../../re/types.h"

struct StdWString {
    u32 allocator;  // std::allocator<wchar_t>: empty, never read
    u16 *ptr;
    u32 len;
    u32 res;
};
static_assert(sizeof(StdWString) == 16, "MSVC6 basic_string is 16 bytes");

// All __thiscall (self in ECX), written as __fastcall with an unused EDX.
extern "C" {

// 0x405280: _Copy(n): a new block for n characters (n | 31 of capacity), keeping the old ones;
// the old block released (freed if s was its only owner or it was frozen, else a count dropped).
void __fastcall StdWString_Copy(StdWString *s, void *edx, u32 n);

// 0x47b750: assign(const wstring &x, pos, n): x's characters from pos, at most n. Shares x's
// block when that is all of x and x's count is below 0xfe; assigning part of itself erases the
// tail and then the head in place. Returns s.
StdWString *__fastcall StdWString_AssignSub(StdWString *s, void *edx, const StdWString *x, u32 pos, u32 n);

// 0x47b970: assign(n, ch): n copies of ch (n of npos throws as too long). Returns s.
StdWString *__fastcall StdWString_AssignFill(StdWString *s, void *edx, u32 n, u32 ch);

// 0x47ba50: _Freeze(): gives a shared s its own copy, then marks s's block (when it has one)
// frozen.
void __fastcall StdWString_Freeze(StdWString *s, void *edx);

// 0x47bad0: _Grow(n, trim): makes room for n characters in a block s owns alone; returns whether
// there are characters to write (0 when n is 0, after emptying s).
u8 __fastcall StdWString_Grow(StdWString *s, void *edx, u32 n, u8 trim);

// 0x47bc00: _Split(): gives a shared s its own copy.
void __fastcall StdWString_Split(StdWString *s, void *edx);

// The second copy (_Nullstr 0x700624):

// 0x4f21d0: assign(const wstring &x, pos, n): x's characters from pos (past x's length: _Xran), at most n. Shares x's block (its count
// raised) when n is all of x, not 0, and x's count is below 0xfe; assigning part of itself erases the tail (from pos + n, at most the wide
// npos at 0x700628) and then the head in place. Returns s.
StdWString *__fastcall WStdString_AssignSub(StdWString *s, void *edx, const StdWString *x, u32 pos, u32 n);

// 0x4f39e0: _Grow(n, trim): room for n characters in a block s owns alone (above 0x7ffffffd: _Xlen). A shared block: with n 0 one count
// less and s emptied, else _Copy(n). Else n 0: emptied (trim: the block freed) or ended at 0 (an existing block). Else with trim, a
// block above 31 or below n is freed first and a new one made (_Copy(n)); without trim, only a block below n is replaced (_Copy(n)).
// Returns whether there are characters to write (0 when n is 0).
u8 __fastcall WStdString_Grow(StdWString *s, void *edx, u32 n, u8 trim);

// 0x4f6090: _Copy(n): a new block of n | 31 characters (n above 0x7ffffffd: n; the count asked for is (res + 2) * 2, a negative res + 2
// counting as 0) gets the len old characters; the old block is released (one count less, or freed); the count byte 0, res set and the
// length kept, ended there.
void __fastcall WStdString_Copy(StdWString *s, void *edx, u32 n);

// 0x4f6190: _Split(): a shared s gets its own copy: the old block loses a count, s is emptied and assigned the old characters up to their
// first 0 (wcslen).
void __fastcall WStdString_Split(StdWString *s, void *edx);
}
