#include "present.h"

#include <cstdint>

namespace {

AddrCalls g_calls;

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const u32 kWasStillDrawing = 0x8876021c, kWasSurfaceLost = 0x887601ae;   // (the two answers it waits on)
const u32 kBltWait = 0x1000000, kInvalidate = 0x8000000;

using Hook = void(__cdecl *)();
using Blt = u32(__stdcall *)(u8 *, s32 *, u8 *, s32 *, u32, u32);
using Flip = u32(__stdcall *)(u8 *, u8 *, u32);
using WindowRect = s32(__stdcall *)(u32, s32 *);

u32 blt(u8 *engine, s32 *to, s32 *from, u32 flags) {
    u8 *const front = at<u8 *>(engine, 0x10);
    const u32 result = reinterpret_cast<const Blt *>(at<const u8 *>(front, 0) + 0x14)[0](front, to, at<u8 *>(engine, 0x14), from, flags, 0);
    global<u32>(0x00879cd4) = result;
    return result;
}

void run_hook(u32 address) {
    if (const Hook hook = global<Hook>(address)) hook();
}

void full_screen(u8 *engine) {
    const u32 flags = global<u32>(0x008bd508);
    u32 result;
    if ((flags & 1) && !at<u32>(engine, 0x78c)) {
        if (flags & kInvalidate) {
            global<u32>(0x008bd508) = flags & ~kInvalidate;
            do {
                result = blt(engine, nullptr, nullptr, kBltWait);
            } while (result);
        } else {
            result = blt(engine, nullptr, nullptr, ~global<u32>(0x008bcbc8) & 1);
            if (result == kWasStillDrawing || result == kWasSurfaceLost) {
                result = 0;
                global<u32>(0x00879cd4) = 0;
            }
        }
    } else {
        const u32 flag = ~global<u32>(0x008bcbc8) & 1;
        s32 budget = 50000000;
        do {
            u8 *const front = at<u8 *>(engine, 0x10);
            result = reinterpret_cast<const Flip *>(at<const u8 *>(front, 0) + 0x2c)[0](front, nullptr, flag);
            global<u32>(0x00879cd4) = result;
            budget -= result == kWasStillDrawing ? 1 : 50;
        } while ((result == kWasStillDrawing || result == kWasSurfaceLost) && budget > 0);
    }
    if (static_cast<s32>(result) < 0) g_calls.Cd(0x00564650, g_calls.Cd(0x0056a770, result));
}

void windowed(u8 *engine) {
    global<u32>(0x008bd508) &= ~kInvalidate;
    s32 from[4] = {0, 0, global<s32>(0x008bcbd0), global<s32>(0x008bcbd4)};
    s32 window[4], client[4];
    global<WindowRect>(0x006fd390)(global<u32>(0x008bd518), window);
    global<WindowRect>(0x006fd38c)(global<u32>(0x008bd518), client);
    const s32 border = (client[2] - window[2] - client[0] + window[0]) >> 1;
    const s32 x = client[0] + border, y = client[3] - window[3] + window[1] - border;
    s32 to[4] = {window[0] + x, window[1] + y, window[2] + x, window[3] + y};
    u32 result;
    do {
        result = blt(engine, to, from, kBltWait);
    } while (result == kWasStillDrawing || result == kWasSurfaceLost);
}

}  // namespace

void Present_SetCalls(const AddrCall *table, u32 count) { g_calls.Set(table, count); }

extern "C" {

u32 __fastcall Device_Present(u8 *engine) {
    run_hook(0x008bd510);
    const bool full = (at<u8>(engine, 0x780) & 1) != 0;
    if (full)
        full_screen(engine);
    else
        windowed(engine);
    run_hook(0x008bd514);
    const u32 result = global<u32>(0x00879cd4);
    if (!full && result) g_calls.Th(0x0056af20, 0x00879ad0u, 3u, result);
    return global<u32>(0x00879cd4) == 0;
}
}
