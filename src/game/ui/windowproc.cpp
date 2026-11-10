#include "windowproc.h"

#include <cstdint>

namespace {

AddrCalls g_calls;

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

s32 point_x(u32 lparam) { return static_cast<s32>((lparam & 0xffff) << 11) / global<s16>(0x00faa5c4); }
s32 point_y(u32 lparam) { return static_cast<s32>((lparam >> 16) * 0x600) / global<s16>(0x00faa5c0); }

u32 modifiers() {
    const auto keyState = global<u32(__stdcall *)(u32)>(0x006fd324);
    u32 held = 0;
    if (static_cast<s16>(keyState(0x10)) < 0) held = 3;
    if (static_cast<s16>(keyState(0x11)) < 0) held |= 0xc;
    if (static_cast<s16>(keyState(0x12)) < 0) held |= 0x30;
    return held;
}

void key_event(u32 wparam, u32 lparam, u32 gui, u32 fire) {
    const u32 repeat = (lparam & 0x40000000) ? 1 : 0;
    const u16 word = static_cast<u16>(wparam);
    u32 key = wparam;
    if (word == 0x6b || word == 0x6d || word == 0x6a || word == 0x6f || word == 0x6e) {
        key = (wparam & 0xff) << 8;
    } else {
        const u32 mapped = global<u32(__stdcall *)(s32, u32)>(0x006fd344)(static_cast<s16>(word), 2);
        if (static_cast<u16>(mapped)) key = mapped;
        if (lparam & 0x1000000) key = (key & 0xff) << 8;
    }
    const u32 held = modifiers();
    g_calls.Cd(gui, key, (repeat << 8 | held) << 16 | (lparam & 0xffff));
    const u32 heldNow = modifiers();
    g_calls.Th(fire, global<u32>(0x0078718c), key, heldNow);
}

void wheel(u32 wparam, u32 lparam) {
    const s32 y = point_y(lparam), x = point_x(lparam);
    g_calls.Cd(0x00658bcb, static_cast<s32>(static_cast<s16>(wparam >> 16)) / 120, x, y, 0u);
}

}  // namespace

void WindowProc_SetCalls(const AddrCall *table, u32 count) { g_calls.Set(table, count); }

extern "C" {

u32 __stdcall GameWindow_Proc(u32 window, u32 message, u32 wparam, u32 lparam) {
    if (!global<s16>(0x00faa5c4)) g_calls.Cd(0x006590d0, static_cast<u32>(global<u16>(0x007a1968)), static_cast<u32>(global<u16>(0x007a1930)));
    if (!global<u32>(0x007b4b88)) {
        const s32 x = point_x(lparam), y = point_y(lparam);
        switch (message) {
            case 0x200: g_calls.Cd(0x006588ef, x, y); break;
            case 0x201: g_calls.Cd(0x00658af1, 0u, x, y); break;
            case 0x202: g_calls.Cd(0x00658b5b, 0u, x, y); break;
            case 0x204: g_calls.Cd(0x00658af1, 1u, x, y); break;
            case 0x205: g_calls.Cd(0x00658b5b, 1u, x, y); break;
            case 0x207: g_calls.Cd(0x00658af1, 2u, x, y); break;
            case 0x208: g_calls.Cd(0x00658b5b, 2u, x, y); break;
            default: break;
        }
    }
    switch (message) {
        case 0x1c:
            if (wparam) {
                if (const u32 cache = global<u32>(0x008c4b44)) g_calls.Th(0x0058dd60, cache, 1u);
                gWindowproc_7a1a14 |= 8;
                g_calls.Cd(0x00659333);
                if (global<u32>(0x00f82884)) g_calls.Cd(0x0051bd30);
                if (global<u32>(0x007b4b88)) global<u32(__stdcall *)(u32)>(0x006fd30c)(0);
            } else {
                gWindowproc_7a1a14 &= ~8u;
                g_calls.Cd(0x0051b350);
                g_calls.Cd(0x006593c5);
                if (global<u32>(0x00f82884)) g_calls.Cd(0x0051bcf0);
                else g_calls.Th(0x004092a0, 0x00786b68u, 1u, 1u);
            }
            g_calls.Cd(0x0047e9d0, wparam);
            return global<u32(__stdcall *)(u32, u32, u32, u32)>(0x006fd340)(window, 0x1c, wparam, lparam);
        case 0x14:
        case 0x20:
            return 1;
        case 0x100:
        case 0x104:
            key_event(wparam, lparam, 0x00658c38, 0x0040c900);
            return 0;
        case 0x101:
        case 0x105:
            key_event(wparam, lparam, 0x00658c67, 0x0040c990);
            return 0;
        case 0x102: {
            u32 bytes = wparam, unit = 0;
            if (global<s32(__stdcall *)(u32, u32, const u32 *, s32, u32 *, s32)>(0x006fd174)(0, 0, &bytes, -1, &unit, 2) <= 0) return 0;
            if (static_cast<u16>(unit) < 0x20) return 0;
            if (static_cast<u8>(g_calls.Th(0x0041a850, 0x0078b530u))) unit = g_calls.Cd(0x0046b460, unit);
            g_calls.Cd(0x00658c96, unit, (lparam & 0xffff) | ((lparam & 0x40000000) ? 1u << 24 : 0));
            return 0;
        }
        case 0x20a:
            wheel(wparam, lparam);
            return 0;
        case 0x410:
            g_calls.Cd(0x00658c96, wparam, 0u);
            return 0;
        default:
            if (message == global<u32>(0x007a1c5c)) wheel(wparam, lparam);
            return 1;
    }
}
}
