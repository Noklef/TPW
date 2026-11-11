#include "funnytime.h"

#include <cstdint>

#include "../gfx/statsgraph.h"
#include "../ui/dialog7cb25c.h"
#include "../ui/dialog7cb2b8.h"
#include "../world/messagecentre.h"
#include "../world/thingmsg.h"
#include "filetime.h"
#include "savegame.h"
#include "imports.h"
#include "addrcalls.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const u32 kSecsPerRealSec = 15000;  // mFunnySecsPerRealSec's start
const unsigned long long kTicksPerSecond = 10000000;  // a FILETIME's 100 ns ticks

u32 worldTicks() { return at<u32>(global<const u8 *>(0x0080239c), 0x1da70c); }  // the world's quarter seconds

// The funny time span of `ticks` quarter seconds at the clock's rate: the 64-bit product / 4 (_alldiv: signed, so a product past 2^63
// counts back) seconds, wrapping.
FileTime funnySpan(const u8 *clock, u32 ticks) {
    const long long seconds = (long long)((unsigned long long)at<u32>(clock, 0x1c) * ticks) / 4;
    const unsigned long long span = (unsigned long long)seconds * kTicksPerSecond;
    return {(u32)span, (u32)(span >> 32)};
}

FileTime funnyNow(const u8 *clock) {
    const FileTime span = funnySpan(clock, worldTicks());
    FileTime now;
    FileTime_Add(reinterpret_cast<const FileTime *>(clock), nullptr, &now, &span);
    return now;
}

// The date FunnyTime_Update keeps (9 dwords; the last two only handed on).
struct GameDate {
    u32 second, minute, hour, day, month, year, dayOfWeek, rest[2];
};

const FunnyTimeEnv kGameEnv = {&MessageCentre_Post, &Obj7b51f0_ShowDate, GameAddress<void(__cdecl *)(u32)>(0x004a5e10),
                               reinterpret_cast<void(__cdecl *)(u32)>(&StatsGraph_Redraw), reinterpret_cast<void(__cdecl *)(u32)>(&Dialog7cb25c_Refresh)};
const FunnyTimeEnv *g_env = &kGameEnv;

}  // namespace

extern "C" {

void FunnyTime_SetEnv(const FunnyTimeEnv *env) { g_env = env ? env : &kGameEnv; }

s32 __fastcall FunnyTime_Quarter(const u8 *clock) {
    const FileTime now = funnyNow(clock);
    s32 month;  // (unset when the conversion fails, sic)
    s32 result;
    FileTime_ToSystemTime(&now, nullptr, &result, nullptr, reinterpret_cast<u32 *>(&month), nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
    return month / 3;
}

void __fastcall FunnyTime_SyncMonthYear(u8 *clock) {
    FileTime now = funnyNow(clock);
    u32 month;  // (unset when the conversion fails, sic)
    s32 result;
    FileTime_ToSystemTime(&now, nullptr, &result, nullptr, &month, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
    at<u32>(clock, 0x10) = month - 1;
    now = funnyNow(clock);
    FileTime_ToSystemTime(&now, nullptr, &result, &at<u32>(clock, 0x18), nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
}

void __fastcall FunnyTime_Update(u8 *clock) {
    GameDate &date = global<GameDate>(0x007ced58);
    FileTime now = funnyNow(clock);
    s32 result;
    FileTime_ToSystemTime(&now, nullptr, &result, &date.year, &date.month, &date.day, &date.dayOfWeek, &date.hour, &date.minute, &date.second, nullptr);
    date.year -= 0x76c;
    date.month -= 1;
    u8 msg[8];
    if (date.day != at<u32>(clock, 0x14)) {
        ThingMsg0b_Ctor(msg);
        g_env->post(MessageCentre_Instance(), nullptr, msg);
        g_env->showDate(reinterpret_cast<void *>((uintptr_t)0x007b51f0), nullptr, date.second, date.minute, date.hour, date.day, date.month, date.year,
                        date.dayOfWeek, date.rest[0], date.rest[1]);
    }
    if (date.month != at<u32>(clock, 0x10)) {
        ThingMsg0c_Ctor(msg);
        g_env->post(MessageCentre_Instance(), nullptr, msg);
        g_env->newMonth(date.month + 1);
        g_env->redrawGraph(date.month + 1);
        g_env->refreshDialog(date.month + 1);
    }
    now = funnyNow(clock);
    u32 year;  // (unset when the conversion fails, sic)
    FileTime_ToSystemTime(&now, nullptr, &result, &year, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
    if (year != at<u32>(clock, 0x18)) {
        ThingMsg0d_Ctor(msg);
        g_env->post(MessageCentre_Instance(), nullptr, msg);
    }
    at<u32>(clock, 0x10) = date.month;
    at<u32>(clock, 0x14) = date.day;
    at<u32>(clock, 0x18) = year;
}

u8 *__fastcall FunnyTime_Construct(u8 *clock) {
    FileTime *const start = reinterpret_cast<FileTime *>(clock);
    FileTime *const session = start + 1;
    start->low = start->high = 0;
    session->low = session->high = 0;
    at<u32>(clock, 0x1c) = kSecsPerRealSec;
    s32 result;
    FileTime_FromSystemTime(start, nullptr, &result, 2000, 1, 1, 0, 0, 0, 0);
    at<u32>(clock, 0x10) = 0;
    at<s32>(clock, 0x14) = -1;
    at<u32>(clock, 0x18) = 0;
    u16 st[8];  // SYSTEMTIME: year, month, day of the week, day, hour, minute, second, ms
    (*reinterpret_cast<void(__stdcall *const *)(u16 *)>(IMPORT(Kernel32_GetLocalTime)))(st);  // GetLocalTime
    FileTime_FromSystemTime(session, nullptr, &result, st[0], st[1], st[3], st[4], st[5], st[6], st[7]);
    return clock;
}

FileTime *__fastcall FunnyTime_Now(const u8 *clock, void *, FileTime *out, u32 *year, u32 *month, u32 *day,
                                   u32 *dayOfWeek, u32 *hour, u32 *minute, u32 *second, u32 *ms) {
    const FileTime span = funnySpan(clock, worldTicks());
    FileTime now;
    FileTime_Add(reinterpret_cast<const FileTime *>(clock), nullptr, &now, &span);
    s32 result;  // (the original has it written over the stack slot of its argument `second`: no matter)
    FileTime_ToSystemTime(&now, nullptr, &result, year, month, day, dayOfWeek, hour, minute, second, ms);
    *out = now;
    return out;
}

FileTime *__fastcall FunnyTime_SpanOf(const u8 *clock, void *, FileTime *out, u32 ticks) {
    *out = funnySpan(clock, ticks);
    return out;
}

FileTime *__fastcall FunnyTime_Elapsed(const u8 *clock, void *, FileTime *out) {
    FileTime now;
    FunnyTime_Now(clock, nullptr, &now, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
    FileTime_Sub(&now, nullptr, out, reinterpret_cast<const FileTime *>(clock));
    return out;
}

FileTime *__fastcall FunnyTime_SpanSince(const u8 *clock, void *, FileTime *out, u32 ticks) {
    *out = funnySpan(clock, worldTicks() - ticks);
    return out;
}

u32 __fastcall GameClock_Serialize(u8 *clock, void *, Stream *stream, u32 save, s32 version, u32) {
    if (version < 0 || version > 1)
        return 0;
    if (!Save_Field(stream, save, clock, 8, (u32)(uintptr_t)&gFunnytime_75c7e0) || !Save_Field(stream, save, clock + 8, 8, (u32)(uintptr_t)&gFunnytime_75c7d0) ||
        !Save_Field(stream, save, clock + 0x10, 4, (u32)(uintptr_t)&gFunnytime_75c7bc) || !Save_Field(stream, save, clock + 0x14, 4, (u32)(uintptr_t)&gFunnytime_75c7a8) ||
        !Save_Field(stream, save, clock + 0x1c, 4, (u32)(uintptr_t)&gFunnytime_75c790))
        return 1;
    return 0;
}
}

s32 FunnyTime_DaysSince(const FileTime *date) {
    FileTime now, age;
    FunnyTime_Now(global<const u8 *>(0x0080239c) + 0x2a0, nullptr, &now, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
                  nullptr);
    FileTime_Sub(&now, nullptr, &age, date);
    return (s32)((long long)((unsigned long long)age.high << 32 | age.low) / 864000000000LL);  // (_alldiv by a day's ticks)
}
