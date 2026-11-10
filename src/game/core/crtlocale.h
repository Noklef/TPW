// The C library's setlocale and the search behind it (MSVC6's setlocale.c and getqloc.c).
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x67c250: strncmp: up to n bytes, to the first's end: -1, 0 or 1 (the bytes unsigned).
s32 __cdecl CRT_strncmp(const char *a, const char *b, u32 n);

// 0x68d800: __fix_grouping: "3;2" to its numbers: the digits less '0', the semicolons taken out, others stepped over (bytes signed).
void __cdecl CRT_fix_grouping(char *text);
// 0x68d840: __free_lconv_mon: with lconv not null and its first text (+0xc) not the "C" locale's own 0xfb1940, the seven texts +0xc to +0x24 freed.
void __cdecl CRT_free_lconv_mon(u32 *lconv);
// 0x68e680: LcidFromHexString: base 16, without a check of the digits.
u32 __cdecl CRT_LcidFromHexString(const char *text);

// 0x68db70: strcspn.
u32 __cdecl CRT_strcspn(const char *text, const char *set);

// 0x684210: _strcats: appends the n strings after n to out.
void __cdecl CRT_strcats(char *out, s32 n, ...);

// 0x684260: __lc_strtolc: "language_country.codepage" into names (language +0, country +0x40, code page +0x80, 0x88 bytes). -1: not of that form.
s32 __cdecl CRT_lc_strtolc(char *names, const char *locale);

// 0x684370: __lc_lctostr: the names back into a string.
void __cdecl CRT_lc_lctostr(char *out, const char *names);

// 0x68e6c0: GetPrimaryLen: the letters that start the text.
s32 __cdecl CRT_GetPrimaryLen(const char *text);
// 0x68e480: TestDefaultLanguage.
u32 __cdecl CRT_TestDefaultLanguage(u32 lcid, u32 primary);

// 0x68e510: IsThisWindowsNT: 1 when GetVersionExA answers and the platform is 2.
s32 __cdecl CRT_IsThisWindowsNT();

// 0x68dd50: TranslateName: a search of table (8 byte entries: the key's pointer, the name itself) from 0 to high; a match points *name at the entry's name. The last comparison, 1 with none.
s32 __cdecl CRT_TranslateName(void *table, s32 high, const char **name);

// 0x68ddb0: GetLcidFromLangCountry. 0x68e120: GetLcidFromLanguage. 0x68e260: GetLcidFromCountry: the state for the callback (0x68de40, 0x68e180, 0x68e2a0) that EnumSystemLocalesA calls, whose result in the state [0xfb18f4] is cleared unless good enough.
void __cdecl CRT_GetLcidFromLangCountry();
void __cdecl CRT_GetLcidFromLanguage();
void __cdecl CRT_GetLcidFromCountry();

// 0x68e340: GetLcidFromDefault: the user's locale.
void __cdecl CRT_GetLcidFromDefault();

// 0x68e360: ProcessCodePage: the number of the code page's name; none or "ACP" the locale's ANSI code page, "OCP" its OEM one. 0 when the locale has none.
u32 __cdecl CRT_ProcessCodePage(const char *name);

// 0x68dbb0: __get_qualified_locale: the locale that names ask for, its ids (language, country, code page; id may be null) and full names (out may be null). 0: no such locale.
s32 __cdecl CRT_get_qualified_locale(const char *names, u16 *id, char *out);

// 0x684010: _expandlocale: the locale that text names in out, through a cache of one ([0x780e18] the text, [0x780ea0] the name). 0: no such locale; otherwise the cache's name ("C": out). category is not used.
char *__cdecl CRT_expandlocale(const char *text, char *out, u16 *id, u32 *code_page, s32 category);

// 0x683ef0: _setlocale_get_all: the categories as one string in [0x780dd4], or when they are all the same, that one and [0x780dd4] freed. No check of the allocation (sic).
char *__cdecl CRT_setlocale_get_all();

// 0x683d90: _setlocale_set_cat: a category's locale: a copy, its ids and its init routine, undone when that fails. 0: failed.
char *__cdecl CRT_setlocale_set_cat(s32 category, const char *locale);

// 0x683ac0: setlocale, under lock 0x13 with [0xfb3ca4] counted, waiting for [0xfb3ca8] to be 0.
char *__cdecl CRT_setlocale(s32 category, const char *locale);
// 0x68e450: TestDefaultCountry: 0 for a language id (the low word) in the ten of [0x781a20], else 1.
u32 __cdecl CRT_TestDefaultCountry(u32 lcid);
// 0x68e2a0: CountryEnumProc: the search's country against this locale's name; 1 goes on with the enumeration.
s32 __stdcall CRT_CountryEnumProc(const char *hex);
// 0x68de40: LangCountryEnumProc: the search's language and country against this locale's names (abbreviated or full, a primary language
// prefix, the default country and language); 1 goes on with the enumeration.
s32 __stdcall CRT_LangCountryEnumProc(const char *hex);
// 0x68e180: LanguageEnumProc: the search's language against this locale's name; 1 goes on.
s32 __stdcall CRT_LanguageEnumProc(const char *hex);
// 0x68e550: crtGetLocaleInfoA: the 27 locales of the table at 0x781a38 (0x2c bytes each, sorted by lcid) answer types 1, 3, 7, 0xb, 0x1001,
// 0x1002 and 0x1004 themselves (the text cut to len - 1); anything else goes to GetLocaleInfoA.
s32 __stdcall CRT_crtGetLocaleInfoA(u32 lcid, u32 what, char *out, s32 room);

}
