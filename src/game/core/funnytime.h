// The game's calendar clock ("funny time": game seconds run at mFunnySecsPerRealSec to the real one; saved with the world globals by
// WorldGlobals_Serialize 0x4d7a70): reimplementation of its serializer 0x4f7f30. It holds mFunnyTimeStart (a FILETIME at +0, 2000-01-01),
// mSessionStart (8 bytes at +8), mMonthAtLastUpdate (+0x10), mDayAtLastUpdate (+0x14) and mFunnySecsPerRealSec (+0x1c). Also its
// constructor 0x4f7e80, the funny time now 0x4f8690 and its spans 0x4f8730, 0x4f8800 and 0x4f88b0; the world's clock is at +0x2a0 of
// the world ([0x80239c]), whose +0x1da70c counts quarter seconds.
#pragma once

#include "../../../re/types.h"

struct FileTime;
struct Stream;

// What FunnyTime_Update calls outside the clock, so that it can run outside the game (the benches point it at recorders).
struct FunnyTimeEnv {
    u32(__fastcall *post)(u8 *centre, void *, u8 *msg);                                                        // 0x40fb10, MessageCentre_Post
    void(__fastcall *showDate)(void *ui, void *, u32, u32, u32, u32 day, u32 month, u32 year, u32, u32, u32);  // 0x481560, Obj7b51f0_ShowDate
    void(__cdecl *newMonth)(u32 month);                                                                        // 0x4a5e10 (agent C's, unported)
    void(__cdecl *redrawGraph)(u32 month);                                                                     // 0x49b520, StatsGraph_Redraw
    void(__cdecl *refreshDialog)(u32 month);                                                                   // 0x4a0770, Dialog7cb25c_Refresh
};

extern "C" {
// FunnyTime_Update's calls go to env (nullptr: the game's).
void FunnyTime_SetEnv(const FunnyTimeEnv *env);

// 0x4f8770 (thiscall; name invented): the month of the funny time now (FunnyTime_Now's sum, FileTime_ToSystemTime) / 3 (signed, toward 0):
// the year's quarter, 4 in December. A failed conversion leaves the month the stack's (sic).
s32 __fastcall FunnyTime_Quarter(const u8 *clock);

// 0x4f85a0 (thiscall; name invented): +0x10 = the month now - 1, +0x18 = the year now (not written when the conversion fails; +0x10 is then the
// stack's, sic). Works the time out twice, as the original does.
void __fastcall FunnyTime_SyncMonthYear(u8 *clock);

// 0x4f8260 (thiscall; name invented): the clock's tick: the date now into the globals 0x7ced58 (second, minute, hour, day, month, year, day of the
// week, 9 dwords in all), year less 1900, month less 1. A new day (0x7ced64 not +0x14): message 0xb posted (MessageCentre_Instance; the message on the
// stack), Obj7b51f0_ShowDate (0x7b51f0) with the nine dwords by value. A new month (0x7ced68 not +0x10): message 0xc posted, then month + 1 to 0x4a5e10,
// StatsGraph_Redraw and Dialog7cb25c_Refresh (cdecl; the last two ignore it). A new year (the year now, in full, not +0x18): message 0xd posted. Then
// +0x10, +0x14 and +0x18 updated (the year the stack's when its conversion fails, sic).
void __fastcall FunnyTime_Update(u8 *clock);

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

// (Not hooked: inlined by its users, Object_AgeDays 0x4dd670 and Staff_AgeDays 0x505b70.) Whole days (signed, toward 0; the low dword) of
// the world's funny time now (FunnyTime_Now, no fields) less date.
s32 FunnyTime_DaysSince(const FileTime *date);

#include "gamedata.h"

// Globals by address, by hand (.docs/09-globals.md; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(char[0x18], gFunnytime_75c790, 0x0075c790, 0x18);
GAME_DATA(char[0x14], gFunnytime_75c7a8, 0x0075c7a8, 0x14);
GAME_DATA(char[0x14], gFunnytime_75c7bc, 0x0075c7bc, 0x14);
GAME_DATA(char[0x10], gFunnytime_75c7d0, 0x0075c7d0, 0x10);
GAME_DATA(char[0x10], gFunnytime_75c7e0, 0x0075c7e0, 0x10);
