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
// 0x6a67f0 / 0x69a7a0: _Locimp::_Makexloc / _Makewloc: _Makeloc's shape for the other facets of char / of wchar_t (collate, messages, money_get / _put, moneypunct
// both ways, time_get / _put; for wchar_t also ctype, num_get / _put, numpunct and codecvt), each new from the _Locinfo when loc is null, else loc's own or the classic one.
void __cdecl Locimp_Makexloc(void *info, u32 cats, Locimp *imp, u8 **loc);
void __cdecl Locimp_Makewloc(void *info, u32 cats, Locimp *imp, u8 **loc);
// 0x6a6fc0 / 0x69ff10 (pops 4; collate<char> / <wchar_t> by their shape, names invented, unconfirmed): refs, the vtable 0x709fc4 / 0x709c24, _Getcoll at +8 under a _Locinfo.
u32 *__fastcall CollateC_Ctor(u32 *self, void *, u32 refs);
u32 *__fastcall CollateW_Ctor(u32 *self, void *, u32 refs);
// 0x6ab0e0 / 0x6a4490 (pops 8; names invented, unconfirmed): the facets of vtable 0x70a044 / 0x709ef4 from a _Locinfo: refs, +8 the _Timevec's block taken from _Gettnames
// (the emptied _Timevec's free of null follows). Returns self.
u32 *__fastcall Facet70a044_Ctor(u32 *self, void *, const void *info, u32 refs);
u32 *__fastcall Facet709ef4_Ctor(u32 *self, void *, const void *info, u32 refs);
// use_facet of those (0x6abe80, 0x6a5720; the _Locinfo of the name at 0x75c74c) and 0x6a54a0 of the time facet of vtable 0x709ed4 (TimeW_Ctor's days and months, widened).
u32 *__cdecl UseFacet_70a044(u8 **loc, u32 unused, u32 flag);
u32 *__cdecl UseFacet_709ef4(u8 **loc, u32 unused, u32 flag);
u32 *__cdecl UseFacet_TimeW(u8 **loc, u32 unused, u32 flag);
// 0x6a4520 (thiscall, pops 4): _Locinfo::_Gettnames: the block of _Gettnames in the _Timevec out, which is returned.
u32 *__fastcall Locinfo_Gettnames(const void *self, void *, u32 *out);

// Destructor bodies of the facets of these vtables (names invented, by the table, unconfirmed; thiscall): the vtable set, the blocks +8 and +0xc freed by Mem_Free, then the locale facet's vtable 0x709b44: 0x6a4460, 0x6a74c0, 0x6ab0b0.
void __fastcall Facet709ed4_Dtor(u32 *self, void *);
void __fastcall Facet709fac_Dtor(u32 *self, void *);
void __fastcall Facet70a024_Dtor(u32 *self, void *);
// The same with the block +8 alone, by free: 0x6a4770, 0x6ab390.
void __fastcall Facet709ef4_Dtor(u32 *self, void *);
void __fastcall Facet70a044_Dtor(u32 *self, void *);
// 0x6a4510 (thiscall): _Timevec's destructor: free of its block.
void __fastcall Timevec_Dtor(void **self, void *);
// 0x69ff80: two wide ranges compared by value; a short range that matches is the lesser (name invented, unconfirmed).
s32 __fastcall WideRange_Compare(void *, void *, const u16 *a, const u16 *a_end, const u16 *b, const u16 *b_end);
// 0x6a7270, 0x6ad010: the vtable set back, freed on flag 1 (names by their tables, invented).
u8 *__fastcall Vtbl709b44_DeletingDtor(u8 *self, void *, u32 flags);
// The facets' deleting destructors: the destructor, then freed on flag 1 (names invented, unconfirmed).
u32 *__fastcall Facet709ed4_DeletingDtor(u32 *self, void *, u32 flags);   // 0x6a4440
u32 *__fastcall Facet709fac_DeletingDtor(u32 *self, void *, u32 flags);   // 0x6a74a0
u32 *__fastcall Facet70a024_DeletingDtor(u32 *self, void *, u32 flags);   // 0x6ab090
u32 *__fastcall Facet709ef4_DeletingDtor(u32 *self, void *, u32 flags);   // 0x6a4750
u32 *__fastcall Facet70a044_DeletingDtor(u32 *self, void *, u32 flags);   // 0x6ab370
u32 *__fastcall MoneypunctC_DeletingDtor(u32 *self, void *, u32 flags);   // 0x6a9190
u32 *__fastcall MoneypunctW_DeletingDtor(u32 *self, void *, u32 flags);   // 0x6a23e0
u32 *__fastcall Numpunct_DeletingDtor(u32 *self, void *, u32 flags);      // 0x695be0
u32 *__fastcall WNumpunct_DeletingDtor(u32 *self, void *, u32 flags);     // 0x69caa0
u32 *__fastcall WFacet709c0c_DeletingDtor(u32 *self, void *, u32 flags);  // 0x6a0460
u8 *__fastcall Vtbl70a0a8_DeletingDtor(u8 *self, void *, u32 flags);

}

#include "gamedata.h"

// Globals by address, by hand (.docs/09-globals.md; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(const char[0x8], gLocalefacet_709b60, 0x00709b60, 0x8);
GAME_DATA(const char[0x8], gLocalefacet_709b68, 0x00709b68, 0x8);
GAME_DATA(const char[0x4], gLocalefacet_709d08, 0x00709d08, 0x4);
GAME_DATA(const char[0x4], gLocalefacet_709d0c, 0x00709d0c, 0x4);
GAME_DATA(const char[0x88], gLocalefacet_709df0, 0x00709df0, 0x88);
GAME_DATA(const char[0x15], gLocalefacet_MissingFacet_75c734, 0x0075c734, 0x15);
GAME_DATA(const char[2], gLocalefacet_C_75c74c, 0x0075c74c, 0x2);
GAME_DATA(u32, gLocalefacet_7ced3c, 0x007ced3c, 0x4);
GAME_DATA(u32, gLocalefacet_7ced40, 0x007ced40, 0x4);
GAME_DATA(u32, gLocalefacet_7ced48, 0x007ced48, 0x4);
GAME_DATA(u32, gLocalefacet_f827e0, 0x00f827e0, 0x4);
GAME_DATA(u32, gLocalefacet_f827e4, 0x00f827e4, 0x4);
GAME_DATA(u32, gLocalefacet_f827e8, 0x00f827e8, 0x4);
GAME_DATA(u32, gLocalefacet_fb1da0, 0x00fb1da0, 0x4);
GAME_DATA(u32, gLocalefacet_fb1da4, 0x00fb1da4, 0x4);
GAME_DATA(u32, gLocalefacet_fb1da8, 0x00fb1da8, 0x4);
GAME_DATA(u32, gLocalefacet_fb1dac, 0x00fb1dac, 0x4);
GAME_DATA(u32, gLocalefacet_fb1db0, 0x00fb1db0, 0x4);
GAME_DATA(u32, gLocalefacet_fb1db4, 0x00fb1db4, 0x4);
GAME_DATA(u32, gLocalefacet_fb1db8, 0x00fb1db8, 0x4);
GAME_DATA(u32, gLocalefacet_fb1dbc, 0x00fb1dbc, 0x4);
GAME_DATA(u32, gLocalefacet_fb1dc0, 0x00fb1dc0, 0x4);
GAME_DATA(u32, gLocalefacet_fb1dc4, 0x00fb1dc4, 0x4);
GAME_DATA(u8 *, gLocalefacet_fb1dcc, 0x00fb1dcc, 0x4);
GAME_DATA(u8 *, gLocalefacet_fb1dd0, 0x00fb1dd0, 0x4);
GAME_DATA(u32, gLocalefacet_fb1dd4, 0x00fb1dd4, 0x4);
GAME_DATA(u32, gLocalefacet_fb1dd8, 0x00fb1dd8, 0x4);
GAME_DATA(u32, gLocalefacet_fb1ddc, 0x00fb1ddc, 0x4);
GAME_DATA(u32, gLocalefacet_fb1de0, 0x00fb1de0, 0x4);
GAME_DATA(u32, gLocalefacet_fb1de4, 0x00fb1de4, 0x4);
GAME_DATA(u32, gLocalefacet_fb1de8, 0x00fb1de8, 0x4);
GAME_DATA(u32, gLocalefacet_fb1dec, 0x00fb1dec, 0x4);
GAME_DATA(u32, gLocalefacet_fb1df0, 0x00fb1df0, 0x4);
GAME_DATA(u32, gLocalefacet_fb1df4, 0x00fb1df4, 0x4);
GAME_DATA(u32, gLocalefacet_fb1df8, 0x00fb1df8, 0x4);
GAME_DATA(u32, gLocalefacet_fb1dfc, 0x00fb1dfc, 0x4);
GAME_DATA(u32, gLocalefacet_fb1e00, 0x00fb1e00, 0x4);
GAME_DATA(u32, gLocalefacet_fb1e04, 0x00fb1e04, 0x4);
GAME_DATA(u32, gLocalefacet_fb1e08, 0x00fb1e08, 0x4);
GAME_DATA(u32, gLocalefacet_fb1e0c, 0x00fb1e0c, 0x4);
GAME_DATA(u32, gLocalefacet_fb1e10, 0x00fb1e10, 0x4);
GAME_DATA(u32, gLocalefacet_fb1e14, 0x00fb1e14, 0x4);
GAME_DATA(u32, gLocalefacet_fb1e18, 0x00fb1e18, 0x4);
GAME_DATA(u32, gLocalefacet_fb1e1c, 0x00fb1e1c, 0x4);
GAME_DATA(u32, gLocalefacet_fb1e20, 0x00fb1e20, 0x4);
GAME_DATA(u32, gLocalefacet_fb1e24, 0x00fb1e24, 0x4);
GAME_DATA(u32, gLocalefacet_fb1e28, 0x00fb1e28, 0x4);
GAME_DATA(u32, gLocalefacet_fb1e2c, 0x00fb1e2c, 0x4);
GAME_DATA(u32, gLocalefacet_fb1e30, 0x00fb1e30, 0x4);
GAME_DATA(u32, gLocalefacet_fb1e34, 0x00fb1e34, 0x4);
GAME_DATA(u32, gLocalefacet_fb1e38, 0x00fb1e38, 0x4);
GAME_DATA(u32, gLocalefacet_fb1e3c, 0x00fb1e3c, 0x4);
GAME_DATA(u32, gLocalefacet_fb1e40, 0x00fb1e40, 0x4);
GAME_DATA(u32, gLocalefacet_fb1e44, 0x00fb1e44, 0x4);
GAME_DATA(u32, gLocalefacet_fb1e48, 0x00fb1e48, 0x4);
GAME_DATA(u32, gLocalefacet_fb1e4c, 0x00fb1e4c, 0x4);
GAME_DATA(u32, gLocalefacet_fb1e50, 0x00fb1e50, 0x4);
GAME_DATA(u32, gLocalefacet_fb1e54, 0x00fb1e54, 0x4);
GAME_DATA(u32, gLocalefacet_fb1e58, 0x00fb1e58, 0x4);
GAME_DATA(u32, gLocalefacet_fb1e5c, 0x00fb1e5c, 0x4);
GAME_DATA(u32, gLocalefacet_fb1e60, 0x00fb1e60, 0x4);
GAME_DATA(u32, gLocalefacet_fb1e64, 0x00fb1e64, 0x4);
GAME_DATA(u32, gLocalefacet_fb1e7c, 0x00fb1e7c, 0x4);
GAME_DATA(u32, gLocalefacet_fb1e80, 0x00fb1e80, 0x4);
GAME_DATA(u32, gLocalefacet_fb1e84, 0x00fb1e84, 0x4);
GAME_DATA(u32, gLocalefacet_fb1e88, 0x00fb1e88, 0x4);
GAME_DATA(u32, gLocalefacet_fb1e8c, 0x00fb1e8c, 0x4);
GAME_DATA(u32, gLocalefacet_fb1e90, 0x00fb1e90, 0x4);
GAME_DATA(u32, gLocalefacet_fb1e94, 0x00fb1e94, 0x4);
GAME_DATA(u32, gLocalefacet_fb1e98, 0x00fb1e98, 0x4);
GAME_DATA(u32, gLocalefacet_fb1e9c, 0x00fb1e9c, 0x4);
GAME_DATA(u32, gLocalefacet_fb1ea0, 0x00fb1ea0, 0x4);
GAME_DATA(u32, gLocalefacet_fb1ea4, 0x00fb1ea4, 0x4);
GAME_DATA(u32, gLocalefacet_fb1ea8, 0x00fb1ea8, 0x4);
GAME_DATA(u32, gLocalefacet_fb1eac, 0x00fb1eac, 0x4);
GAME_DATA(u32, gLocalefacet_fb1eb0, 0x00fb1eb0, 0x4);
GAME_DATA(u32, gLocalefacet_fb1eb4, 0x00fb1eb4, 0x4);
GAME_DATA(u32, gLocalefacet_fb1eb8, 0x00fb1eb8, 0x4);
GAME_DATA(u32, gLocalefacet_fb1ebc, 0x00fb1ebc, 0x4);
GAME_DATA(u32, gLocalefacet_fb1ec0, 0x00fb1ec0, 0x4);
GAME_DATA(u32, gLocalefacet_fb1ec4, 0x00fb1ec4, 0x4);
GAME_DATA(u32, gLocalefacet_fb1ec8, 0x00fb1ec8, 0x4);
GAME_DATA(u32, gLocalefacet_fb1ecc, 0x00fb1ecc, 0x4);
GAME_DATA(u32, gLocalefacet_fb1ed0, 0x00fb1ed0, 0x4);
GAME_DATA(u32, gLocalefacet_fb1ed4, 0x00fb1ed4, 0x4);
GAME_DATA(u32, gLocalefacet_fb1ed8, 0x00fb1ed8, 0x4);
GAME_DATA(u32, gLocalefacet_fb275c, 0x00fb275c, 0x4);

// The vtables hold only ports: tpw.exe's loader points their slots at them (.docs/09-globals.md).
GAME_DATA(void *[6], Numpunct_vtable, 0x00709aac, 0x18);
GAME_DATA(void *[7], NumPut_vtable, 0x00709acc, 0x1c);
GAME_DATA(void *[10], NumGet_vtable, 0x00709aec, 0x28);
GAME_DATA(void *[1], LocaleFacet_vtable, 0x00709b44, 0x4);
GAME_DATA(void *[10], Class709bbc_vtable, 0x00709bbc, 0x28);
GAME_DATA(void *[4], WFacet709c0c_vtable, 0x00709c0c, 0x10);
GAME_DATA(void *[4], CollateW_vtable, 0x00709c24, 0x10);
GAME_DATA(void *[6], WNumpunct_vtable, 0x00709c3c, 0x18);
GAME_DATA(void *[7], WNumGet_vtable, 0x00709c5c, 0x1c);
GAME_DATA(void *[10], Class709f2c_vtable, 0x00709f2c, 0x28);
GAME_DATA(void *[10], Class709f5c_vtable, 0x00709f5c, 0x28);
GAME_DATA(void *[3], Facet709f8c_vtable, 0x00709f8c, 0xc);
GAME_DATA(void *[3], Facet709f9c_vtable, 0x00709f9c, 0xc);
GAME_DATA(void *[4], Facet709fac_vtable, 0x00709fac, 0x10);
