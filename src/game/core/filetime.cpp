// The 64-bit time: see filetime.h.
#include "filetime.h"

#include <cstdint>

namespace {

typedef unsigned long long u64;

template <typename T>
T at(u32 address) {
    return reinterpret_cast<T>(static_cast<std::uintptr_t>(address));
}
const FileTimeEnv kGameEnv = {
    at<s32(__stdcall *const *)(const FileTime *, u16 *)>(0x006fd0bc),
    at<s32(__stdcall *const *)(const u16 *, FileTime *)>(0x006fd150),
};
const FileTimeEnv *g_env = &kGameEnv;

u64 get(const FileTime *t) { return (u64)t->high << 32 | t->low; }
void put(FileTime *t, u64 v) {
    t->low = (u32)v;
    t->high = (u32)(v >> 32);
}

}  // namespace

void FileTime_SetEnv(const FileTimeEnv *e) { g_env = e ? e : &kGameEnv; }

extern "C" {

FileTime *__fastcall FileTime_Add(const FileTime *t, void *, FileTime *out, const FileTime *b) {
    put(out, get(t) + get(b));
    return out;
}

FileTime *__fastcall FileTime_Sub(const FileTime *t, void *, FileTime *out, const FileTime *b) {
    put(out, get(t) - get(b));
    return out;
}

u32 __fastcall FileTime_Greater(const FileTime *t, void *, const FileTime *b) { return get(t) > get(b); }

FileTime *__fastcall FileTime_Assign(FileTime *t, void *, const FileTime *b) {
    t->low = b->low;
    t->high = b->high;
    return t;
}

s32 *__fastcall FileTime_FromSystemTime(FileTime *t, void *, s32 *out, u32 year, u32 month, u32 day, u32 hour,
                                        u32 minute, u32 second, u32 ms) {
    const FileTime saved = *t;
    u16 st[8] = {(u16)year, (u16)month, 0, (u16)day, (u16)hour, (u16)minute, (u16)second, (u16)ms};
    if ((*g_env->fromSystemTime)(st, t) && t->high < 0x80000000u) {
        *out = 0;
    } else {
        *t = saved;
        *out = -1;
    }
    return out;
}

s32 *__fastcall FileTime_ToSystemTime(const FileTime *t, void *, s32 *out, u32 *year, u32 *month, u32 *day,
                                      u32 *dayOfWeek, u32 *hour, u32 *minute, u32 *second, u32 *ms) {
    u16 st[10];
    if (!(*g_env->toSystemTime)(t, st)) {
        *out = -1;
        return out;
    }
    if (year) *year = st[0];
    if (month) *month = st[1];
    if (day) *day = st[3];
    if (dayOfWeek) *dayOfWeek = st[2];
    if (hour) *hour = st[4];
    if (minute) *minute = st[5];
    if (second) *second = st[6];
    if (ms) *ms = st[7];
    *out = 0;
    return out;
}

s32 *__fastcall FileTime_FromSpan(FileTime *t, void *, s32 *out, s32 days, s32 hours, s32 minutes, s32 seconds,
                                  s32 ms) {
    const u32 total = (((((u32)days * 24 + hours) * 60 + minutes) * 60 + seconds) * 1000) + ms;
    put(t, (u64)((long long)(s32)total * 10000));
    *out = 0;
    return out;
}

}  // extern "C"
