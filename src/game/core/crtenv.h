// The C library's environment, its comparison and wide conversion through Win32, and the time zone (MSVC6's getenv, __crtsetenv, __crtCompareStringA, wcstombs, _tzset).
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x68f060: __crtCompareStringA: CompareStringW over the converted strings, or CompareStringA where the system has no W ([0xfb1910]: found on first use; 0: 0). An empty one against a single byte answers from the code page's lead bytes.
s32 __cdecl CRT_crtCompareStringA(u32 locale, u32 flags, const char *a, s32 a_length, const char *b, s32 b_length, u32 code_page);

// 0x68b530: _mbsnbicoll: n bytes, case ignored, in the locale: <0, 0, >0; 0x7fffffff when the comparison fails.
s32 __cdecl CRT_mbsnbicoll(const char *a, const char *b, u32 n);

// 0x691540: _mbschr: strchr without a multibyte code page. With one, under lock 0x19, double byte characters compared as lead << 8 | trail; a lead byte at the end: 0.
const char *__cdecl CRT_mbschr(const char *text, u32 value);

// 0x68b5f0: _strdup.
char *__cdecl CRT_strdup(const char *text);

// 0x68f540: findenv: the index of name in [0xfb1380], or minus the entries' count.
s32 __cdecl CRT_findenv(const char *name, u32 length);

// 0x68f5c0: copy_environ: a new array of copies; no memory: _amsg_exit(9).
char **__cdecl CRT_copy_environ(char **environment);

// 0x68f330: __crtsetenv: "name=value" in [0xfb1380] (an empty value removes), with primary also in the process. The string is kept, not copied. Bad option or no memory: -1.
s32 __cdecl CRT_crtsetenv(char *option, s32 primary);

// 0x68b570: __wtomb_environ: the wide environment [0xfb1388] into the narrow one. No memory or a failed conversion: -1.
s32 __cdecl CRT_wtomb_environ();

// 0x67fc20: getenv: the value of name (case ignored), or 0.
char *__cdecl CRT_getenv(const char *name);

// 0x68ef70: wcsncnt: the units to and with the end, within n; n when it is not there.
u32 __cdecl CRT_wcsncnt(const u16 *text, u32 n);
// 0x688f10 (name invented, unconfirmed; LCMapStringW's): the units before the end, at most n. Past n units the next is read, and either way n is the answer.
u32 __cdecl CRT_wcsnlen(const u16 *text, u32 n);
// 0x689760: _setenvp: the block at [0xfb1354] made into the table of copies at [0xfb1380], without the names that start with '='. Each failed malloc _amsg_exit(9). The block freed and [0xfb1354] cleared.
void __cdecl CRT_setenvp();

// 0x68ed80: _wcstombs_lk: without a locale ([0xfb1440] 0) the units up to 0xff; otherwise WideCharToMultiByte, by characters when the first try does not fit. A character that cannot convert: errno 0x2a, -1.
u32 __cdecl CRT_wcstombs_lk(char *to, const u16 *from, u32 n);

// 0x68ed00: wcstombs: under lock 0x13 when [0xfb3ca4] counts writers of the locale, otherwise as a reader of it, [0xfb3ca8].
u32 __cdecl CRT_wcstombs(char *to, const u16 *from, u32 n);

// 0x687d80: _tzset: from "TZ" (3 letters, hours[:minutes[:seconds]], 3 letters; only when it changed), without one from GetTimeZoneInformation.
void __cdecl CRT_tzset();

}
