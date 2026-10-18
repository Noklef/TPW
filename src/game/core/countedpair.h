// The 0x24-byte element of counted strings whose clears and deleters are in buffers.h (CountedPair_*): a narrow std::string at +0, a wide one at
// +0x10 and a dword at +0x20 (what it holds is not known yet; the code at 0x4ea5xx-0x4ebaxx builds them and hands them to container inserts).
// Reimplementation of its constructors 0x4ea740 and 0x4e6410, its assignment 0x4f5860, the insert of a vector of them, 0x4f5560, and their
// uninitialised copy / fill and construct helpers (0x4f67b0, 0x4f67f0, 0x4f7410).
#pragma once

#include "../../../re/types.h"
#include "stdstring.h"
#include "stdwstring.h"

struct CountedPair {
    StdString narrow;  // +0
    StdWString wide;   // +0x10
    u32 value;         // +0x20
};
static_assert(sizeof(CountedPair) == 0x24, "two strings and a dword");

extern "C" {

// 0x4ea740 (thiscall; a std::wstring by value, pops 16; name unconfirmed): narrow empty and wide assigned the string (StdWString_AssignSub from 0,
// npos), each taking the string's allocator byte; value 0. The string is then destroyed: its block's count byte lowered, or the block freed
// 2 bytes before the characters when the count is 0 or 0xff. Returns the pair.
CountedPair *__fastcall CountedPair_CtorWide(CountedPair *p, void *edx, StdWString s);

// 0x4e6410 (thiscall; x, pops 4; name unconfirmed): the copy constructor: both strings start empty with the low byte of x's address as their
// allocator byte (sic), then are assigned x's characters up to their first 0 (as from a C string: the narrow one through StdString_Grow, the
// wide one past max_size (Const7ffffffd_4f6080) throwing _Xlen); value = x's. Returns the pair.
CountedPair *__fastcall CountedPair_CopyCtor(CountedPair *p, void *edx, const CountedPair *x);

// 0x4f5860 (thiscall; x, pops 4; name unconfirmed): the assignment: each string assigned x's characters up to their first 0 (as from a C
// string; the wide one's _Grow inlined: a shared block loses a count and, with characters to write, gets a fresh copy; an own block above 31
// or below the length is replaced), value = x's. Returns the pair.
CountedPair *__fastcall CountedPair_Assign(CountedPair *p, void *edx, const CountedPair *x);

// 0x4f5560 (thiscall on a vector of pairs {+4 first, +8 last, +0xc end}; where, n, x, pops 12; name unconfirmed): insert(where, n, x), MSVC6's:
// without room for n more, a block of size + max(n, size) pairs (a negative count allocates 0) gets the head, n copies of x and the tail
// (copy-constructed; a null slot skipped), the old pairs are destroyed (CountedPair_Clear) and their block freed; else with fewer than n after
// where, the tail is copied n up, the copies of x past the old end constructed and the old tail's slots assigned x; else the last n are copied up,
// the rest shifted back (assignment) and the n slots from where assigned x. x may be one of the vector's own pairs (read after the moves, sic).
void __fastcall CountedPairVector_InsertN(u8 *v, void *edx, u32 where, u32 n, const CountedPair *x);

// 0x4f67b0 (stdcall, pops 12; name unconfirmed): _Ucopy: the pairs of [first, last) copy-constructed from to on, a null slot skipped (placement
// new's check). Returns the end.
u32 __stdcall CountedPair_UninitCopy(u32 first, u32 last, u32 to);

// 0x4f67f0 (stdcall, pops 12; name unconfirmed): _Ufill: n copies of x constructed from to on, null slots skipped.
void __stdcall CountedPair_UninitFill(u32 to, u32 n, const CountedPair *x);

// 0x4f7410 (cdecl; name unconfirmed): the allocator's construct(p, x): x copy-constructed at p unless p is null.
void __cdecl CountedPair_Construct(CountedPair *p, const CountedPair *x);
}
