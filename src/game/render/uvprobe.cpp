#include "uvprobe.h"

#include <cstdint>
#include <cstring>

#include "../core/gameheap.h"
#include "../core/vcall.h"
#include "../gfx/texture.h"
#include "../math/x87.h"
#include "../frontend/lobbysky.h"
#include "../gfx/texcache.h"
#include "ddopen.h"
#include "d3dsetup.h"
#include "../core/staticobjects.h"

namespace {

AddrCalls g_calls;

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

template <typename T> T &at(void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

u32 address(const void *p) { return (u32)(uintptr_t)p; }

u32 bits(float x) {
    u32 b;
    std::memcpy(&b, &x, 4);
    return b;
}

u8 *device() { return global<u8 *>(0x00879ab0); }

// A line of the texture: its edges, the frame next to them, the middle's lines by turns.
u32 stripe(u32 line, u32 edge, u32 frame, u32 even, u32 odd) {
    if (line < 0x22 || line > 0x5d)
        return edge;
    if (line == 0x22 || line == 0x5d)
        return frame;
    return line & 1 ? odd : even;
}

void fillTexture(u8 *settings, u8 *texture, bool across) {
    u8 *const pixels = reinterpret_cast<u8 *>((uintptr_t)g_calls.Th(0x00590d40, address(texture)));
    if (global<u32>(0x00701e18 + 4 * at<u32>(settings, 4)) == 2) {
        u16 *out = reinterpret_cast<u16 *>(pixels);
        for (u32 y = 0; y < 0x80; ++y) {
            for (u32 x = 0; x < 0x80; ++x)
                *out++ = (u16)stripe(across ? x : y, reinterpret_cast<u16 &>(global<u32>((u32)(uintptr_t)&gView_8bcbc8 + 0x8fc)), reinterpret_cast<u16 &>(global<u32>((u32)(uintptr_t)&gView_8bcbc8 + 0x904)), reinterpret_cast<u16 &>(global<u32>((u32)(uintptr_t)&gView_8bcbc8 + 0x90c)), reinterpret_cast<u16 &>(global<u32>((u32)(uintptr_t)&gView_8bcbc8 + 0x914)));
        }
        return;
    }
    u32 edge, frame, even, odd;
    if (global<u32>(0x008bd524) == 4) {
        edge = 0xff0000;
        frame = 0xff00;
        even = 0xff;
        odd = 0xffffff;
    } else {
        edge = global<u32>((u32)(uintptr_t)&gView_8bcbc8 + 0x8fc);
        frame = global<u32>((u32)(uintptr_t)&gView_8bcbc8 + 0x904);
        even = global<u32>((u32)(uintptr_t)&gView_8bcbc8 + 0x90c);
        odd = global<u32>((u32)(uintptr_t)&gView_8bcbc8 + 0x914);
    }
    u32 *out = reinterpret_cast<u32 *>(pixels);
    for (u32 y = 0; y < 0x80; ++y) {
        for (u32 x = 0; x < 0x80; ++x)
            *out++ = stripe(across ? x : y, edge, frame, even, odd);
    }
}

// The place drawn, 60 lines read back and classified.
s32 tryPlace(u8 *settings, float place, bool across, u32 *lines) {
    const X87Reg scaled = X87Reg(place) * 0.0078125f;   // 0x701e74
    const u32 outer = bits((float)(scaled - -0.75f));     // 0x701e78
    const u32 inner = bits((float)(scaled - -0.25f));    // 0x701e7c
    vcall<u32>(device(), 0x44, 0x42800000u, 0xffffffffu, 0u, inner, inner, outer, outer);
    for (u32 i = 0; i < 0x3c; ++i) {
        if (across)
            vcall<u32>(device(), 0x40, i + 2, 0u, address(lines + i));
        else
            vcall<u32>(device(), 0x40, 0u, i + 2, address(lines + i));
    }
    return (s32)g_calls.Th(0x00583ba0, address(settings), address(lines));
}

void probe(u8 *settings, bool across, u32 result) {
    g_calls.Th(0x0058dd60, gTexcache_8c4b44, 1u);
    g_calls.Cd(0x00583720);
    u8 *const block = static_cast<u8 *>(GameHeap_Alloc(0x60));
    u8 *const texture = block ? Texture_Ctor(block, nullptr, 1) : nullptr;
    if (!texture)
        return;
    const u32 saved = global<u32>((u32)(uintptr_t)&gView_8bcbc8 + 0x8b4);
    g_calls.Th(0x00590a80, address(texture), 0x80u, 0u);
    at<u32>(texture, 8) = 0x51b0000;
    fillTexture(settings, texture, across);
    g_calls.Th(0x0058dd60, gTexcache_8c4b44, 1u);
    g_calls.Th(0x0058c9f0, gTexcache_8c4b44, address(texture), 0u);
    g_calls.Th(0x0058cf40, gTexcache_8c4b44);
    vcall<u32>(device(), 8, 0x1069256u);
    const u32 handle = at<u32>(texture, 8) & 0x1000 ? at<u32>(texture, 0x1c) : 0;
    vcall<u32>(device(), 0xc, 0u, handle);
    vcall<u32>(device(), 0x18, 0u, 0u, 0u);
    global<u32>((u32)(uintptr_t)&gTexSettings_8bd550 + 0x40f4) = 0;

    u32 lines[0x3c];
    float place = 0.0f, step = 0.5f;
    float firstFit = 0.5f, lastFit = 0.0f;
    bool none = true, first = true;
    for (u32 i = 0; i < 0xc; ++i) {
        const s32 answer = tryPlace(settings, place, across, lines);
        if (answer > 0) {
            place = (float)(X87Reg(step) + place);
        } else {
            if (answer == 0) {
                lastFit = place;
                none = false;
                if (first) {
                    firstFit = place;
                    first = false;
                }
            }
            place = (float)(X87Reg(place) - step);
        }
        step = (float)(X87Reg(step) * 0.5f);   // 0x701e70
    }
    if (none) {
        at<u32>(settings, result) = 0;
    } else {
        place = 0.0f;
        step = 0.5f;
        float upFit = 0.0f;
        bool foundUp = false;
        for (u32 i = 0; i < 0xc; ++i) {
            const s32 answer = tryPlace(settings, place, across, lines);
            if (answer > 0) {
                place = (float)(X87Reg(step) + place);
            } else if (answer == 0) {
                upFit = place;
                foundUp = true;
                place = (float)(X87Reg(step) + place);
            } else {
                place = (float)(X87Reg(place) - step);
            }
            step = (float)(X87Reg(step) * 0.5f);
        }
        at<float>(settings, result) = foundUp ? (float)((X87Reg(upFit) + lastFit) * 0.5f) : (float)((X87Reg(lastFit) + firstFit) * 0.5f);
    }
    g_calls.Th(0x0058dd60, gTexcache_8c4b44, 1u);
    g_calls.Th(0x00590a70, address(texture));
    GameHeap_Free(texture);
    global<u32>((u32)(uintptr_t)&gView_8bcbc8 + 0x8b4) = saved;
}

void note(const char *text) {
    if (global<u32>(0x008bd508) & 0x2000000)
        g_calls.Cd(0x005645e0, address(text));
}

void setOffsets(u8 *settings, u32 value) {
    at<u32>(settings, 0x40ec) = value;
    at<u32>(settings, 0x40e8) = value;
}

}  // namespace

void UvProbe_SetCalls(const AddrCall *table, u32 count) { g_calls.Set(table, count); }

void __fastcall UvProbe_Down(u8 *settings) { probe(settings, false, 0x40ec); }

void __fastcall UvProbe_Across(u8 *settings) { probe(settings, true, 0x40e8); }

void __fastcall UvProbe_Detect(u8 *settings) {
    at<u32>(settings, 0x40e4) = 0;
    at<u32>(settings, 0x40e0) = 0;
    at<u32>(settings, 0x40ec) = 0;
    at<u32>(settings, 0x40e8) = 0;
    if (global<u32>(0x008bd524) != 1)
        return;
    u8 *const direct = global<u8 *>((u32)(uintptr_t)&gD3dsetup_879a20 + 0x4);
    const u32 vendor = at<u32>(direct, 0x428);
    const bool permedia = vendor == 0x104c && at<u32>(direct, 0x42c) == 0x3d07;
    const bool tnt = vendor == 0x10de && at<u32>(direct, 0x42c) == 0x20;
    if (vendor == 0x1002 && at<u32>(direct, 0x42c) == 0x4c42)
        global<u32>((u32)(uintptr_t)&gView_8bcbc8) |= 2;
    if (at<u32>(direct, 0x428) == 0x1002 && at<u32>(direct, 0x42c) == 0x524c)
        global<u32>((u32)(uintptr_t)&gView_8bcbc8) |= 2;
    bool early = false;
    if (tnt) {
        u32 hardware[0xfc / 4], software[0xfc / 4];
        hardware[0] = 0xfc;
        software[0] = 0xfc;
        u8 *const surface = at<u8 *>(global<u8 *>(0x00879da8), 8);
        const s32 result = comcall<s32>(surface, 0xc, hardware, software);
        if (result < 0)
            g_calls.Cd(0x00564650, g_calls.Cd(0x0056a770, (u32)result));
        early = !((hardware[0xf4 / 4] >> 4) & 1);
    }
    if (permedia) {
        note("NOTICE: This is an Permedia2, overriding detection");   // (sic)
        global<u32>((u32)(uintptr_t)&gView_8bcbc8) |= 2;
        setOffsets(settings, 0x3ee66666);   // (0.45)
        return;
    }
    if (early) {
        note("NOTICE: This is an early version of the NVidia TNT, overriding detection");
        global<u32>((u32)(uintptr_t)&gView_8bcbc8) |= 2;
    } else if (global<u32>((u32)(uintptr_t)&gView_8bcbc8) & 0x8000002) {
        note("NOTICE: This is either a Rage Pro or Riva128, overriding");
    } else {
        g_calls.Th(0x00584310, address(settings));
        g_calls.Th(0x00583e90, address(settings));
        return;
    }
    setOffsets(settings, 0x3e800000);   // (0.25)
}
