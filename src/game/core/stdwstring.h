// The game's std::wstring: MSVC6 <xstring> basic_string<wchar_t> (the IME's strings), whose
// out-of-line members the compiler emitted at 0x405280 and 0x47b750-0x47bcd5. Reimplementation
// of those members; the wide string's others are already ported elsewhere: _Tidy (the counted
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
}
