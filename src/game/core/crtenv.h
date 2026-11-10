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

// 0x689b00: __crtGetEnvironmentStringsA: a malloc'd copy of the process's environment block, from the W strings through WideCharToMultiByte (code page 0) or the
// A ones; [0xfb1660] keeps which (0: found on first use, 1 W, 2 A). The system's block freed. Nothing, no memory or a failed conversion: 0.
char *__cdecl CRT_crtGetEnvironmentStringsA();

// 0x68ed80: _wcstombs_lk: without a locale ([0xfb1440] 0) the units up to 0xff; otherwise WideCharToMultiByte, by characters when the first try does not fit. A character that cannot convert: errno 0x2a, -1.
u32 __cdecl CRT_wcstombs_lk(char *to, const u16 *from, u32 n);

// 0x68ed00: wcstombs: under lock 0x13 when [0xfb3ca4] counts writers of the locale, otherwise as a reader of it, [0xfb3ca8].
u32 __cdecl CRT_wcstombs(char *to, const u16 *from, u32 n);

// 0x687d80: _tzset: from "TZ" (3 letters, hours[:minutes[:seconds]], 3 letters; only when it changed), without one from GetTimeZoneInformation.
void __cdecl CRT_tzset();

// 0x67fbf0: getenv: _getenv_lk (CRT_getenv) under lock 0xc.
char *__cdecl CRT_getenv_locked(const char *name);

}

// The time zone as GetTimeZoneInformation answers it (TIME_ZONE_INFORMATION).
struct CrtTimeZone {
    s32 bias;
    u16 standard_name[32];
    u16 standard_date[8];   // a SYSTEMTIME
    s32 standard_bias;
    u16 daylight_name[32];
    u16 daylight_date[8];
    s32 daylight_bias;
};

#include "gamedata.h"

// Globals by address, by hand (.docs/09-globals.md; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(const char[4], CRT_text_TZ_708da0, 0x00708da0, 0x4);
GAME_DATA(s32, CRT_timezone, 0x007813d8, 0x4);
GAME_DATA(s32, CRT_daylight, 0x007813dc, 0x4);
GAME_DATA(s32, CRT_dstbias, 0x007813e0, 0x4);
GAME_DATA(char[2][0x40], CRT_tzname_buffers, 0x007813e8, 0x80);
GAME_DATA(char *[2], CRT_tzname, 0x00781468, 0x8);
GAME_DATA(char **, CRT_environ, 0x00fb1380, 0x4);
GAME_DATA(char **, CRT_initenv, 0x00fb1384, 0x4);
GAME_DATA(u16 **, CRT_wenviron, 0x00fb1388, 0x4);
GAME_DATA(u32, CRT_tzapiused, 0x00fb1458, 0x4);
GAME_DATA(CrtTimeZone, CRT_tzinfo, 0x00fb1460, 0xac);
GAME_DATA(char *, CRT_lastTZ, 0x00fb150c, 0x4);
GAME_DATA(u32, CRT_GetEnvironmentStrings_use_fb1660, 0x00fb1660, 0x4);
GAME_DATA(u32, CRT_CompareString_use_fb1910, 0x00fb1910, 0x4);
