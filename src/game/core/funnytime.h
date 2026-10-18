// The game's calendar clock ("funny time": game seconds run at mFunnySecsPerRealSec to the real one; saved with the world globals by
// WorldGlobals_Serialize 0x4d7a70): reimplementation of its serializer 0x4f7f30. It holds mFunnyTimeStart (a FILETIME at +0, 2000-01-01),
// mSessionStart (8 bytes at +8), mMonthAtLastUpdate (+0x10), mDayAtLastUpdate (+0x14) and mFunnySecsPerRealSec (+0x1c). Also its
// constructor 0x4f7e80, the funny time now 0x4f8690 and its spans 0x4f8730, 0x4f8800 and 0x4f88b0; the world's clock is at +0x2a0 of
// the world ([0x80239c]), whose +0x1da70c counts quarter seconds.
#pragma once

#include "../../../re/types.h"

struct FileTime;
struct Stream;

extern "C" {
// 0x4f7e80 (thiscall): mFunnyTimeStart = 2000-01-01 (FileTime_FromSystemTime; on its failure 0), mSessionStart = the local time
// (GetLocalTime, then the same; on its failure 0), +0x10 = 0, +0x14 = -1, +0x18 = 0, mFunnySecsPerRealSec = 15000. Returns clock.
u8 *__fastcall FunnyTime_Construct(u8 *clock);

// 0x4f8690 (thiscall; out, &year, &month, &day, &day of the week, &hour, &minute, &second, &ms, each may be 0): *out = mFunnyTimeStart
// + (mFunnySecsPerRealSec * the world's ticks (+0x1da70c, unsigned 64-bit) / 4 (signed) seconds, wrapping); the fields as
// FileTime_ToSystemTime gives them (none written when it fails). Returns out.
FileTime *__fastcall FunnyTime_Now(const u8 *clock, void *edx, FileTime *out, u32 *year, u32 *month, u32 *day, u32 *dayOfWeek,
                                   u32 *hour, u32 *minute, u32 *second, u32 *ms);

// 0x4f8730 (thiscall; out, ticks): *out = the funny time span of `ticks` quarter seconds: mFunnySecsPerRealSec * ticks (unsigned
// 64-bit) / 4 (signed) seconds, wrapping. Returns out.
FileTime *__fastcall FunnyTime_SpanOf(const u8 *clock, void *edx, FileTime *out, u32 ticks);

// 0x4f8800 (thiscall; out): *out = FunnyTime_Now (no fields) less mFunnyTimeStart. Returns out.
FileTime *__fastcall FunnyTime_Elapsed(const u8 *clock, void *edx, FileTime *out);

// 0x4f88b0 (thiscall; out, ticks): *out = the span (as FunnyTime_SpanOf) of the world's ticks less `ticks` (wrapping). Returns out.
FileTime *__fastcall FunnyTime_SpanSince(const u8 *clock, void *edx, FileTime *out, u32 ticks);

// 0x4f7f30 (thiscall; stream, save, version, unused): 0 for a version outside 0..1, else the five fields saved or loaded as a serializer does
// (Save_Field; 1 at the first that fails). The load reads the last through Save_ReadU32, which takes the same steps.
u32 __fastcall GameClock_Serialize(u8 *clock, void *, Stream *stream, u32 save, s32 version, u32 unused);
}
