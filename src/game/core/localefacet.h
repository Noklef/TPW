// The C++ library's std::locale facets of char and _Locimp::_Makeloc, which gives a _Locimp its facets (MSVC6's names; the module's own: NumGet_Ctor and the others below are facets' constructors).
#pragma once

#include "../../../re/types.h"
#include "locimp.h"

extern "C" {

// 0x694710 (pops 4, the _Locinfo not read): ctype<char>::_Init: +8 the 4 dwords of _Getctype, under the lock. The first table: kept at 0xfb1dd8, freed at exit (0x5cfc80), and +0x14 (the table is this facet's to delete) cleared.
void __fastcall CtypeChar_Init(u32 *self, void *, const void *info);

// 0x694780 / 0x695c40 (pops 4): num_put's / num_get's constructors of char: refs, the vtable, and a _Locinfo("C") made and destroyed.
u32 *__fastcall NumPut_Ctor(u32 *self, void *, u32 refs);
u32 *__fastcall NumGet_Ctor(u32 *self, void *, u32 refs);

// 0x695a30 (pops 4, the _Locinfo not read): numpunct<char>::_Init: the decimal point and the separator (bytes +0xc, +0xd) from the current lconv, its grouping (+8) and "false", "true" (+0x10, +0x14) copied.
void __fastcall Numpunct_Init(u32 *self, void *, const void *info);

// 0x699410: a copy of the text from Mem_Alloc (the second argument of this cdecl routine is not read).
char *__cdecl Numpunct_Copy(const char *text);

// 0x698c40 (pops 8, the _Locinfo not read): codecvt<char, char, int>'s constructor: refs, the vtable, _Getcvt at +8.
u32 *__fastcall Codecvt_Ctor(u32 *self, void *, const void *info, u32 refs);

// 0x6991f0 / 0x699440: use_facet of numpunct<char> / codecvt<char, char, int>: the locale's own facet; a missing one from the classic locale (flag's low byte set), made once and freed at exit; else bad_cast thrown with "missing locale facet".
u32 *__cdecl UseFacet_Numpunct(u8 **loc, u32 unused, u32 flag);
u32 *__cdecl UseFacet_Codecvt(u8 **loc, u32 unused, u32 flag);

// 0x693e20 (pops 4): _Locinfo::_Getname: a string at out, its first use, copied from the name at +0x30. Returns out.
StdString *__fastcall Locinfo_GetName(const u8 *self, void *, StdString *out);

// 0x694120: _Locimp::_Makeloc: for the categories in cats (2: ctype and codecvt, 8: numeric), each facet new from the _Locinfo when loc is null, else loc's own or the classic one; then _Makexloc and _Makewloc, the categories added, the name from _Locinfo. Returns imp.
Locimp *__cdecl Locimp_Makeloc(void *info, u32 cats, Locimp *imp, u8 **loc);
}
