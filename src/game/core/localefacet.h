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

// 0x69b490 / 0x69cb00 (pops 4): num_get's / num_put's constructors of wchar_t: as those of char. 0x69b460 (pops 8, the first not read): codecvt<wchar_t, char, int>'s constructor: refs, the vtable 0x709ce4, _Getcvt at +8.
u32 *__fastcall WNumGet_Ctor(u32 *self, void *, u32 refs);
u32 *__fastcall WNumPut_Ctor(u32 *self, void *, u32 refs);
u32 *__fastcall WCodecvt_Ctor(u32 *self, void *, const void *, u32 refs);

// 0x695910 / 0x69c680 (pops 4): numpunct<char> / <wchar_t>'s constructor: refs, the vtable, then _Init (inlined) under a _Locinfo of the name at 0x75c74c.
u32 *__fastcall Numpunct_Ctor(u32 *self, void *, u32 refs);
u32 *__fastcall WNumpunct_Ctor(u32 *self, void *, u32 refs);
// 0x69c7e0 (pops 4, the _Locinfo not read): numpunct<wchar_t>::_Init: the decimal point and the separator (words at +0xc, +0xe) through btowc, signed; the grouping as bytes; "false" and "true" as words, each through btowc.
void __fastcall WNumpunct_Init(u32 *self, void *, const void *info);
// 0x6a49e0 (name invented, unconfirmed): a new block holding the text, end included, each byte through btowc.
u16 *__cdecl Facet_WidenedCopy(const char *text);
// The narrow facet of vtable 0x709fac (names invented, by the table, unconfirmed): 0x6a7290 (pops 4) its constructor: refs, the vtable, _Init under a _Locinfo; 0x6a7370 (pops 4, the _Locinfo not read) _Init: +8 the text at 0x709d0c, +0xc that at 0x709d08. 0x6a74f0, 0x6a9200 (pops 4): the constructors of vtables 0x709f9c and 0x709f8c: refs, the vtable, a _Locinfo.
u32 *__fastcall Facet709fac_Ctor(u32 *self, void *, u32 refs);
void __fastcall Facet709fac_Init(u32 *self, void *, const void *info);
u32 *__fastcall Facet709f9c_Ctor(u32 *self, void *, u32 refs);
u32 *__fastcall Facet709f8c_Ctor(u32 *self, void *, u32 refs);
// The wide facet of vtable 0x709c0c (names invented, by the table, unconfirmed): 0x6a01d0 (pops 4) its constructor: refs, the vtable, _Init under a _Locinfo; 0x6a02d0 (pops 4, the _Locinfo not read) _Init: +8 the text at 0x709d0c, +0xc that at 0x709d08, as words through btowc; 0x6a0480 its destructor body. 0x6a04b0 (pops 4): the constructor of vtable 0x709bfc: refs, the vtable, a _Locinfo.
u32 *__fastcall WFacet709c0c_Ctor(u32 *self, void *, u32 refs);
void __fastcall WFacet709c0c_Init(u32 *self, void *, const void *info);
void __fastcall WFacet709c0c_Tidy(u32 *self, void *);
u32 *__fastcall WFacet709bfc_Ctor(u32 *self, void *, u32 refs);

// 0x695c00 / 0x69cac0: numpunct's destructor bodies: the vtable, the three texts freed, the base vtable 0x709b44.
void __fastcall Numpunct_Tidy(u32 *self, void *);
void __fastcall WNumpunct_Tidy(u32 *self, void *);

// 0x695a30 (pops 4, the _Locinfo not read): numpunct<char>::_Init: the decimal point and the separator (bytes +0xc, +0xd) from the current lconv, its grouping (+8) and "false", "true" (+0x10, +0x14) copied.
void __fastcall Numpunct_Init(u32 *self, void *, const void *info);

// 0x699410: a copy of the text from Mem_Alloc (the second argument of this cdecl routine is not read).
char *__cdecl Numpunct_Copy(const char *text);

// 0x698c40 (pops 8, the _Locinfo not read): codecvt<char, char, int>'s constructor: refs, the vtable, _Getcvt at +8.
u32 *__fastcall Codecvt_Ctor(u32 *self, void *, const void *info, u32 refs);

// 0x6991f0 / 0x699440: use_facet of numpunct<char> / codecvt<char, char, int>: the locale's own facet; a missing one from the classic locale (flag's low byte set), made once and freed at exit; else bad_cast thrown with "missing locale facet".
u32 *__cdecl UseFacet_Numpunct(u8 **loc, u32 unused, u32 flag);
u32 *__cdecl UseFacet_Codecvt(u8 **loc, u32 unused, u32 flag);

// The same for more facets (names invented, by their vtables' shapes, unconfirmed): 0x6a4d70 / 0x6a5180 moneypunct<wchar_t, true / false> (0x2c bytes, MoneypunctW_Init),
// 0x6ab6d0 / 0x6aba80 moneypunct<char, true / false> (MoneypunctC_Init), each with the international byte at +0x28 and its base's vtable first; 0x6a5320 the wide facet of
// vtable 0x709bec (8 bytes); 0x6a58d0 the wide codecvt of vtable 0x709ce4 (_Getcvt at +8); 0x6abc20 the time facet of vtable 0x70a024 (days and months of the "C" _Locinfo).
u32 *__cdecl UseFacet_MoneypunctWIntl(u8 **loc, u32 unused, u32 flag);
u32 *__cdecl UseFacet_MoneypunctW(u8 **loc, u32 unused, u32 flag);
u32 *__cdecl UseFacet_MoneypunctCIntl(u8 **loc, u32 unused, u32 flag);
u32 *__cdecl UseFacet_MoneypunctC(u8 **loc, u32 unused, u32 flag);
u32 *__cdecl UseFacet_709bec(u8 **loc, u32 unused, u32 flag);
u32 *__cdecl UseFacet_CodecvtW(u8 **loc, u32 unused, u32 flag);
u32 *__cdecl UseFacet_TimeC(u8 **loc, u32 unused, u32 flag);

// 0x6aa200 / 0x6a3470 (pops 8; names invented, by their tables' shape, unconfirmed): the time facet of vtable 0x70a024 / 0x709ed4 made from a _Locinfo: refs at +4; its string
// at +0 assigned _Getdays' text (freed) when there is one, +0x10 _Getmonths', and copies of the two (empty: _Nullstr's; an unset string: the texts at 0x709e78 and 0x709df0) at
// +8 and +0xc, new blocks of bytes / of btowc's units. A null block is written through as in the original. Returns self.
u32 *__fastcall TimeC_Ctor(u32 *self, void *, u8 *info, u32 refs);
u32 *__fastcall TimeW_Ctor(u32 *self, void *, u8 *info, u32 refs);

// 0x693e20 (pops 4): _Locinfo::_Getname: a string at out, its first use, copied from the name at +0x30. Returns out.
StdString *__fastcall Locinfo_GetName(const u8 *self, void *, StdString *out);

// 0x694120: _Locimp::_Makeloc: for the categories in cats (2: ctype and codecvt, 8: numeric), each facet new from the _Locinfo when loc is null, else loc's own or the classic one; then _Makexloc and _Makewloc, the categories added, the name from _Locinfo. Returns imp.
Locimp *__cdecl Locimp_Makeloc(void *info, u32 cats, Locimp *imp, u8 **loc);
}
