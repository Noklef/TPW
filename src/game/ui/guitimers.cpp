#include "guitimers.h"

#include <cstdint>

#include "../core/gameheap.h"
#include "guimanager.h"
#include "window.h"

namespace {

template <typename T> T &at(void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

u8 *&entries(u8 *table) { return at<u8 *>(table, 4); }
s32 count(u8 *table) { return at<s32>(table, 0); }
// entry i's field k (0 window, 1 message, 2 a, 3 b, 4 the time due), worked out again for each use as the original does
u32 &field(u8 *table, s32 i, u32 k) { return at<u32>(entries(table) + i * 0x14, 4 * k); }
}

extern "C" {

u32 __fastcall GuiTimers_Init(u8 *table, void *, s32 n) {
    if (entries(table) || n >= 0x200) return 0;
    entries(table) = static_cast<u8 *>(GameHeap_Alloc((u32)(n * 0x14)));
    if (!entries(table)) return 0;
    at<s32>(table, 0) = n;
    for (s32 i = 0; i < n; ++i) field(table, i, 4) = 0;
    return 1;
}

void __fastcall GuiTimers_Free(u8 *table) {
    if (entries(table)) GameHeap_Free(entries(table));
}

u32 __fastcall GuiTimers_Add(u8 *table, void *, u32 window, u32 message, u32 a, u32 b, u32 delay) {
    u32 added = 0;
    if (!entries(table) || delay == 0 || delay >= 60000) return added;
    for (s32 i = 0; i < count(table) && !added; ++i) {
        if (field(table, i, 4)) continue;
        field(table, i, 4) = GuiClock_Now() + delay;
        field(table, i, 0) = window;
        field(table, i, 1) = message;
        field(table, i, 2) = a;
        field(table, i, 3) = b;
        added = 1;
    }
    return added;
}

u32 __fastcall GuiTimers_Fire(u8 *table) {
    u16 fired = 0;
    if (!entries(table)) return fired;
    for (s32 i = 0; i < count(table); ++i) {
        if (!field(table, i, 4)) continue;
        u8 *const list = entries(table);   // (read before the clock is asked)
        if (at<u32>(list + i * 0x14, 0x10) > GuiClock_Now()) continue;
        u8 *const entry = entries(table) + i * 0x14;
        Window_CallHandler(at<u8 *>(entry, 0), nullptr, at<u32>(entry, 4), at<u32>(entry, 8), at<u32>(entry, 0xc));
        ++fired;
        field(table, i, 4) = 0;
    }
    return fired;
}

u32 __fastcall GuiTimers_Cancel(u8 *table, void *, u32 window) {
    s32 cancelled = -1;
    if (!entries(table)) return (u32)cancelled;
    cancelled = 0;
    for (s32 i = 0; i < count(table); ++i) {
        if (field(table, i, 0) != window) continue;
        field(table, i, 4) = 0;
        ++cancelled;
    }
    return (u32)cancelled;
}

void __fastcall GuiTimers_Reset(u8 *table) {
    if (!entries(table)) return;
    for (s32 i = 0; i < count(table); ++i) field(table, i, 4) = 0;
}
}
