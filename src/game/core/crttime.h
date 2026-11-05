// The C library's time text and fields (MSVC6's own names): _Strftime, its _expandtime and _store_winword, the copies of the locale's names, gmtime and cvtdate.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x6902b0 / 0x6903a0: _Getdays / _Getmonths: a block from malloc with ":short:long" for each of the 7 days / 12 months and an end, or null.
char *__cdecl CRT_Getdays();
char *__cdecl CRT_Getmonths();

// 0x690490: _Gettnames: a block from malloc with the names' table at [0x781f8c] and its strings copied after it (their pointers to the copies), or null. The last, the time picture, gets its place but its text is not copied.
char **__cdecl CRT_Gettnames();

// 0x687d40: __tzset: _tzset once, under lock 0xb.
void __cdecl CRT_tzset_once();

// 0x688300: cvtdate: the transition (first, 1, or last) of a year's daylight time, its year and day of the year and its millisecond of the day, into the globals at 0x781470 / 0x781480.
// By day of the month (type 1, by the week and the day of the week) or by the day itself. The last's time counts [0x7813e0] seconds more and wraps at a day, one millisecond short.
void __cdecl CRT_cvtdate(s32 transition, s32 type, s32 year, s32 month, s32 week, s32 weekday, s32 day, s32 hour, s32 minute, s32 second, s32 millisecond);

// 0x688090: _isindst_lk: whether the struct tm's time is daylight time, by the year's two transitions: made again by cvtdate when the year is not that of both, from the time zone's dates (a date with no year: by the week and the day of the week)
// or with no zone information [0xfb1458] from the first Sunday of April to the last of October, at 2:00. No daylight rule [0x7813dc]: 0.
s32 __cdecl CRT_isindst_lk(const s32 *when);

// 0x6884a0: gmtime: the seconds at *when into the thread's struct tm (made by malloc at its +0x40; if that fails the shared one at 0xfb1518), null for a time before 1970.
s32 *__cdecl CRT_gmtime(const s32 *when);

// 0x690800: _Strftime: the format into `out`, `size` bytes, %x as _expandtime, %#x its alternate form; the names null: the current ones. Returns the bytes (without the end), 0 if they did not fit.
u32 __cdecl CRT_Strftime(char *out, u32 size, const char *format, const s32 *when, char *const *names);

// 0x690950: _expandtime: one conversion; the unknown ones store nothing.
void __cdecl CRT_expandtime(char what, const s32 *when, char **out, u32 *left, char *const *names);

// 0x690ef0: _store_str: the text as far as it fits.
void __cdecl CRT_store_str(const char *text, char **out, u32 *left);

// 0x690f20: _store_num: the number in `digits` digits, or with no leading zeros by [0xfb1930] through _store_number; no room: *left 0.
void __cdecl CRT_store_num(s32 number, u32 digits, char **out, u32 *left);

// 0x690fc0: _store_number: as many digits as fit, leaving one byte; with none, the turn around still swaps the byte before the output with the one at it.
void __cdecl CRT_store_number(s32 number, char **out, u32 *left);

// 0x691030: _store_winword: a Windows date or time picture (dd, MMMM, hh, 'text' and so on) through _expandtime. The picture's one-letter forms ask for no leading zeros, but _expandtime sets that again from the alternate form.
void __cdecl CRT_store_winword(const char *format, const s32 *when, char **out, u32 *left, char *const *names);

// 0x688060: _isindst: _isindst_lk under lock 0xb.
s32 __cdecl CRT_isindst(const s32 *when);

}  // extern "C"
