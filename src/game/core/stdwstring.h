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

// 0x47ae60: basic_string(const wstring &x): x's allocator byte, then empty and assign(x, 0, npos) (inlined: shares x's block as
// AssignSub does). Returns s.
StdWString *__fastcall StdWString_CtorCopy(StdWString *s, void *edx, const StdWString *x);

// 0x47af80: basic_string(n, ch, allocator): the allocator's byte, then empty and assign(n, ch) (inlined). Returns s.
StdWString *__fastcall StdWString_CtorFill(StdWString *s, void *edx, u32 n, u32 ch, const u8 *allocator);

// 0x4f0170 (thiscall (chars, allocator), pops 8; name unconfirmed): basic_string(const wchar_t *, allocator): the allocator's byte, then
// empty and assign(chars, wcslen(chars)) (inlined, as Split's: a block of 31 or more characters made through _Copy for any). Returns s.
StdWString *__fastcall StdWString_CtorChars(StdWString *s, void *edx, const u16 *chars, const u8 *allocator);

// 0x47b600 (thiscall on a vector, which is not read; first, last, dest, pops 12): _Ucopy: each string of [first, last) copied
// (CtorCopy, inlined) to the next 16 bytes from dest, unless dest is null (only skipped: the next is 0x10 on, sic). Returns the end.
StdWString *__fastcall StdWStrings_UninitCopy(void *vector, void *edx, const StdWString *first, const StdWString *last, StdWString *dest);

// 0x47cc10: append(const wstring &x, pos, n): x's characters from pos (past x's length: _Xran), at most n, after s's (n of npos less
// s's length or more: _Xlen); _Grow(length, false) inlined. Returns s.
StdWString *__fastcall StdWString_AppendSub(StdWString *s, void *edx, const StdWString *x, u32 pos, u32 n);

// 0x47b460: erase(pos, n): (past the length: _Xran) s split, then at most n characters from pos removed. Returns s.
StdWString *__fastcall StdWString_Erase(StdWString *s, void *edx, u32 pos, u32 n);

// 0x47d9d0 (cdecl; p, x): std::_Construct: a copy of x made at p (CtorCopy, inlined) unless p is null.
void __cdecl StdWString_Construct(StdWString *p, const StdWString *x);

// 0x47d620 (cdecl; first, last, dest): std::copy: each string of [first, last) assigned (assign(x, 0, npos), inlined) to the next
// from dest. Returns the end.
StdWString *__cdecl StdWStrings_Copy(const StdWString *first, const StdWString *last, StdWString *dest);

// 0x47b320 (thiscall on a vector<wstring> {allocator, first +4, last +8, end +0xc}; first, last, pops 8): erase(first, last): the
// strings from last on assigned down from first (as StdWStrings_Copy), those after the new last released and emptied (_Tidy). Returns
// first.
StdWString *__fastcall StdWStringVector_Erase(u8 *vector, void *edx, StdWString *first, const StdWString *last);

// 0x47ac10 (thiscall on a vector<wstring>; x, pops 4): operator=: nothing onto itself; when x has no more strings, its strings assigned
// over the first ones (assign(x, 0, npos)) and the rest released (CountedBuf_Deleter, core/buffers); within the capacity, as many as
// there are assigned and the rest copied into the room past them (_Construct); past it, all released (_Tidy), the block freed and a new
// one of x's size (a negative one 0) made with copies of x's (_Ucopy, inlined), end = last. Returns vector.
u8 *__fastcall StdWStringVector_Assign(u8 *vector, void *edx, const u8 *x);

// 0x47c970 (thiscall on a vector<wstring>; position, n, x, pops 12): insert(position, n, x). Without room for n (the end less the last,
// shifted signed, compared unsigned): a new block of the size plus the larger of n and the size (a negative count 0) gets copies
// (CtorCopy, inlined; a null slot skipped) of the strings before the position, n of x and the rest; the old ones released (_Tidy) and the
// block freed, last = the new first + n + the old size (read from the freed block's pointers). With room and fewer than n strings after the
// position: those copied n on, the room past them filled with copies of x and x assigned over the ones after the position. Else (n 0:
// nothing) the last n copied past the end, the rest from the position shifted up n (assigned backwards), x assigned over the n from the
// position. Each x is read through its pointer, so an x inside the vector is the string then at that place (sic).
void __fastcall StdWStringVector_InsertN(u8 *vector, void *edx, StdWString *position, u32 n, const StdWString *x);

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
