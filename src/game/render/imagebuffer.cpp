#include "imagebuffer.h"

#include <cstdint>
#include <cstring>

#include "../core/vcall.h"
#include "ddopen.h"

namespace {

AddrCalls g_calls;

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
u32 &global(u32 address) { return *reinterpret_cast<u32 *>((uintptr_t)address); }
u32 address_of(const void *p) { return (u32)(uintptr_t)p; }

// The screen's pixel format into format[8]; false when an answer other than 0 was pushed as an error.
bool screen_format(u32 *format) {
    std::memset(format, 0, 0x20);
    format[0] = 0x20;
    const u32 surface = at<u32>(reinterpret_cast<const void *>((uintptr_t)global(0x00879a24)), 0x10);
    const u32 answer = comcall<u32>(reinterpret_cast<void *>((uintptr_t)surface), 0x54, format);
    global((u32)&gDdopen_879cd4) = answer;
    if ((s32)answer < 0)
        g_calls.Cd(0x00564650, g_calls.Cd(0x0056a770, answer));
    if (const u32 now = global((u32)&gDdopen_879cd4)) {
        g_calls.Th(0x0056af20, 0x00879ad0, 2u, now);
        return false;
    }
    return true;
}

// A channel table of the format for driver kind 1 (into table); else the table at 0x8c05d8.
u32 channel_table(const u32 *format, u8 *table) {
    if (global(0x008bd524) != 1)
        return 0x008c05d8;
    g_calls.Cd(0x00591270, address_of(table), format[0], format[1], format[2], format[3], format[4], format[5], format[6], format[7]);
    return address_of(table);
}

}  // namespace

void ImageBuffer_SetCalls(const AddrCall *table, u32 count) { g_calls.Set(table, count); }

extern "C" {

u32 __fastcall ImageBuffer_Load(u8 *self, void *, const char *name) {
    u32 format[8];
    u8 table[0x1000];
    if (!screen_format(format))
        return 0;
    at<u32>(self, 0xc) = global(0x008bd524) == 1 ? format[3] >> 3 : 4;
    const u32 used = channel_table(format, table);
    at<u32>(self, 0) = g_calls.Cd(0x00574bd0, name, address_of(self + 4), address_of(self + 8), 0u, used, at<u32>(self, 0xc));
    return at<u32>(self, 0) != 0;
}

u32 __fastcall ImageBuffer_Stamp(u8 *self, void *, const char *name, u32, s32 y) {
    if (!at<u32>(self, 0))
        return 1;
    u32 format[8];
    u8 table[0x1000];
    if (!screen_format(format))
        return 0;
    s32 width, height;
    const u32 used = channel_table(format, table);
    const u32 block = g_calls.Cd(0x00574bd0, name, address_of(&width), address_of(&height), 0u, used, at<u32>(self, 0xc));
    if (!block)
        return 1;
    s32 row = at<s32>(self, 8) - height - y, skipped = 0;
    if (row < 0) {
        skipped = -row;
        row = 0;
    }
    const u32 bytes = at<u32>(self, 0xc);
    u8 *to = reinterpret_cast<u8 *>((uintptr_t)(at<u32>(self, 0) + (u32)row * at<u32>(self, 4) * bytes));
    const u8 *from = reinterpret_cast<const u8 *>((uintptr_t)(block + (u32)skipped * (u32)width * bytes));
    s32 place = y;
    for (s32 i = 0; i < height; ++i, ++place) {
        const u32 step = at<u32>(self, 0xc);
        if (place < at<s32>(self, 8))
            std::memcpy(to, from, (u32)width * step);
        from += (u32)width * step;
        to += at<u32>(self, 4) * step;
    }
    g_calls.Cd(0x00579b80, block);
    return 1;
}
}
