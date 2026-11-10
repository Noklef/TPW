// The C++ library's time_get (MSVC6): _Getloctxt, and the members that read a weekday or a month name by it. Names invented, unconfirmed.
#pragma once

#include "../../../re/types.h"
#include "numget.h"

extern "C" {

// 0x6ab3b0, 0x6a4a30 (cdecl): _Getloctxt of char, of wchar_t: names is a list of names each led by its first character (":Sun:Sunday:Mon..."); n plus their
// count is how many there are. The stream is matched against all at once, a character at a time: the answer is the index of the longest that ended at a
// match, -1 when one was a prefix of the input but none ended, -2 when none matched at all. first moves past what matched.
s32 __cdecl TimeGet_Getloctxt(NumGetIter *first, NumGetIter *last, u32 n, const char *names);
s32 __cdecl WTimeGet_Getloctxt(NumGetIter *first, NumGetIter *last, u32 n, const u16 *names);

// 0x6aaa90, 0x6aaaf0, 0x6a3de0, 0x6a3e40 (pops 0x20): do_get_weekday and do_get_monthname of char, of wchar_t: _Getloctxt over the names at +8 (days), +0xc
// (months); the index halved (each name has a short and a long form) to tm_wday (+0x18) or tm_mon (+0x10), else state 2. Returns first at out.
u32 *__fastcall TimeGet_DoGetWeekday(u8 *self, void *, u32 *out, NumGetIter first, NumGetIter last, u8 *ios, u32 *state, u8 *tm);
u32 *__fastcall TimeGet_DoGetMonthname(u8 *self, void *, u32 *out, NumGetIter first, NumGetIter last, u8 *ios, u32 *state, u8 *tm);
u32 *__fastcall WTimeGet_DoGetWeekday(u8 *self, void *, u32 *out, NumGetIter first, NumGetIter last, u8 *ios, u32 *state, u8 *tm);
u32 *__fastcall WTimeGet_DoGetMonthname(u8 *self, void *, u32 *out, NumGetIter first, NumGetIter last, u8 *ios, u32 *state, u8 *tm);
}
