// The 64-bit time (thiscall unless noted): a FILETIME, 100 ns ticks since 1601, with its sums,
// comparisons and conversions to and from a SYSTEMTIME's fields (0x5fc530-0x5fc730) and a span
// built from days to milliseconds (0x5fcc50).
//
// The originals' quirks are kept: FromSystemTime leaves the SYSTEMTIME's day of the week as
// whatever was on the stack (Windows ignores it) and only takes the low 16 bits of each field;
// it undoes a result at or past 2^63 as a failure; FromSpan works out the milliseconds in 32 bits,
// wrapping, so its range check (against 2^63 ticks) can never fail.
#pragma once

#include "../../../re/types.h"

struct FileTime {
    u32 low;
    u32 high;  // +4
};
static_assert(sizeof(FileTime) == 8, "FileTime is 8 bytes");

struct FileTimeEnv {
    s32(__stdcall *const *toSystemTime)(const FileTime *, u16 *);    // 0x6fd0bc FileTimeToSystemTime
    s32(__stdcall *const *fromSystemTime)(const u16 *, FileTime *);  // 0x6fd150 SystemTimeToFileTime
};
// nullptr restores the game's.
void FileTime_SetEnv(const FileTimeEnv *e);

extern "C" {
// 0x5fc530 (&out, &b): out = t + b; returns out.
FileTime *__fastcall FileTime_Add(const FileTime *t, void *edx, FileTime *out, const FileTime *b);
// 0x5fc550 (&out, &b): out = t - b; returns out.
FileTime *__fastcall FileTime_Sub(const FileTime *t, void *edx, FileTime *out, const FileTime *b);
u32 __fastcall FileTime_Greater(const FileTime *t, void *edx, const FileTime *b);  // 0x5fc570: t > b
FileTime *__fastcall FileTime_Assign(FileTime *t, void *edx, const FileTime *b);   // 0x5fc5a0: returns t
// 0x5fc5c0 (&out, year, month, day, hour, minute, second, ms): out 0, or -1 (t unchanged).
s32 *__fastcall FileTime_FromSystemTime(FileTime *t, void *edx, s32 *out, u32 year, u32 month, u32 day, u32 hour,
                                        u32 minute, u32 second, u32 ms);
// 0x5fc660 (&out, &year, &month, &day, &day of week, &hour, &minute, &second, &ms; each may be 0).
s32 *__fastcall FileTime_ToSystemTime(const FileTime *t, void *edx, s32 *out, u32 *year, u32 *month, u32 *day,
                                      u32 *dayOfWeek, u32 *hour, u32 *minute, u32 *second, u32 *ms);
// 0x5fcc50 (&out, days, hours, minutes, seconds, ms): t = that span in ticks; out 0.
s32 *__fastcall FileTime_FromSpan(FileTime *t, void *edx, s32 *out, s32 days, s32 hours, s32 minutes, s32 seconds,
                                  s32 ms);
}
