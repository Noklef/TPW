// The game's std::string: MSVC6 <xstring> basic_string<char>, whose out-of-line members the
// compiler emitted once at 0x45b340-0x45b998 (most other uses are inlined; 103 call _Xlen).
// Reimplementation of those members.
//
// Copy-on-write: the characters sit one byte into a heap block (Mem_Alloc 0x67acc0) whose
// first byte counts the other owners (0: one owner, 0xff: frozen, never shared). A string is
// { allocator (empty), ptr, len, res }; res is the capacity, at least 31 once allocated
// (_MIN_SIZE), and the block is res + 2 bytes. A null ptr reads as _Nullstr (0x6fe5b0).
//
// Library bugs kept (sic):
//  - _Copy(n) copies all len old characters into a block sized for n | 31: a shared string
//    grown to a shorter n overruns the new block when len > (n | 31) + 1.
//  - _Copy's catch(...) retry with the exact size is unreachable: the game's operator new
//    returns null rather than throwing, so it is left out.
#pragma once

#include "../../../re/types.h"

struct StdString {
    u32 allocator;  // std::allocator<char>: empty, never read
    char *ptr;
    u32 len;
    u32 res;
};
static_assert(sizeof(StdString) == 16, "MSVC6 basic_string is 16 bytes");

struct StdStringEnv {
    void *(__cdecl *alloc)(u32 size);  // Mem_Alloc (0x67acc0)
    void(__cdecl *free)(void *block);  // Mem_Free (0x67acb0)
    void(__cdecl *xlen)();             // std::_Xlen (0x6918f0): throws length_error
    void(__cdecl *xran)();             // std::_Xran (0x691ba0): throws out_of_range
};
// The strings' calls go to env (nullptr: the game's).
void StdStrings_SetEnv(const StdStringEnv *env);
// The env's throws, for the wide strings (stdwstring.h), which share them.
void StdStrings_Xlen();
void StdStrings_Xran();

// All __thiscall (self in ECX), written as __fastcall with an unused EDX.
extern "C" {

// 0x45b340: ~basic_string(): _Tidy(true).
void __fastcall StdString_Dtor(StdString *s, void *edx);

// 0x45b380: assign(const string &x, pos, n): x's characters from pos, at most n. Shares x's
// block when that is all of x and x's count is below 0xfe; assigning part of itself erases
// the tail and then the head in place. Returns s.
StdString *__fastcall StdString_AssignSub(StdString *s, void *edx, const StdString *x, u32 pos, u32 n);

// 0x45b590: _Tidy(built): releases the block when built (frees it if s is the only owner or it
// is frozen, else drops a count), then empties s.
void __fastcall StdString_Tidy(StdString *s, void *edx, u8 built);

// 0x45b5e0: assign(const char *chars, n). Returns s.
StdString *__fastcall StdString_AssignChars(StdString *s, void *edx, const char *chars, u32 n);

// 0x45b680: _Eos(n): the length and terminator.
void __fastcall StdString_Eos(StdString *s, void *edx, u32 n);

// 0x45b6a0: _Grow(n, trim): makes room for n characters in a block s owns alone; returns
// whether there are characters to write (0 when n is 0, after emptying s).
u8 __fastcall StdString_Grow(StdString *s, void *edx, u32 n, u8 trim);

// 0x45b7c0: max_size(): 0xfffffffd.
u32 __fastcall StdString_MaxSize(const StdString *s, void *edx);

// 0x45b7d0: _Copy(n): a new block for n characters (n | 31 of capacity), keeping the old ones.
void __fastcall StdString_Copy(StdString *s, void *edx, u32 n);

// 0x45b8d0: _Split(): gives a shared s its own copy.
void __fastcall StdString_Split(StdString *s, void *edx);

// 0x4a9420: basic_string(const basic_string &x), emitted out of line apart from the others (name unconfirmed): the allocator's
// byte copied, s emptied, then assign(x, 0, npos) inlined: as AssignSub, but without its max_size check, and with x's length
// read after s is emptied (so copying s into itself leaves it empty: sic). Returns s.
StdString *__fastcall StdString_CopyCtor(StdString *s, void *edx, const StdString *x);
}
