#include "funnytime.h"

#include <cstdint>

#include "filetime.h"
#include "savegame.h"

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

}  // namespace

extern "C" {

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
    (*reinterpret_cast<void(__stdcall *const *)(u16 *)>(0x006fd164))(st);  // GetLocalTime
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
    if (!Save_Field(stream, save, clock, 8, 0x0075c7e0) || !Save_Field(stream, save, clock + 8, 8, 0x0075c7d0) ||
        !Save_Field(stream, save, clock + 0x10, 4, 0x0075c7bc) || !Save_Field(stream, save, clock + 0x14, 4, 0x0075c7a8) ||
        !Save_Field(stream, save, clock + 0x1c, 4, 0x0075c790))
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
