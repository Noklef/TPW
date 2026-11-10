#include "surfacepool.h"

#include <cstdint>
#include <cstring>

#include "../core/vcall.h"
#include "../render/ddopen.h"

namespace {

template <typename T> T &at(const void *p, s32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
u32 &global32(u32 address) { return *reinterpret_cast<u32 *>((uintptr_t)address); }
u8 *global32p(u32 address) { return *reinterpret_cast<u8 **>((uintptr_t)address); }

}  // namespace

extern "C" {

s32 __cdecl SurfacePool_SideIndex(s32 side) {
    switch (side) {
    case 1:
        return 8;
    case 2:
        return 7;
    case 4:
        return 6;
    case 8:
        return 5;
    case 0x10:
        return 4;
    case 0x20:
        return 3;
    case 0x40:
        return 2;
    case 0x80:
        return 1;
    case 0x100:
        return 0;
    default:
        return -1;
    }
}

u32 __fastcall SurfacePool_Next(u8 *pool, void *, s32 side, u32) {
    const s32 index = SurfacePool_SideIndex(side);
    const u32 turn = at<u32>(pool, 0x80 + 4 * index);
    at<u32>(pool, 0x80 + 4 * index) = (turn - 1) & 1;
    return at<u32>(pool, 8 * index + 4 * (s32)turn);
}
}

namespace {
AddrCalls g_poolCalls;

// The result kept at [0x879cd4]; a failure's text logged (DxResult_Message, EngineDebug_Log).
u32 kept(u32 result) {
    global32((u32)&gDdopen_879cd4) = result;
    if ((s32)result < 0) {
        g_poolCalls.Cd(0x00564650, g_poolCalls.Cd(0x0056a770, result));
    }
    return global32((u32)&gDdopen_879cd4);
}
}  // namespace

void SurfacePool_SetCalls(const AddrCall *table, u32 count) { g_poolCalls.Set(table, count); }

namespace {
AddrCalls g_levelsCalls;
u32 address(const void *p) { return (u32)(uintptr_t)p; }
}  // namespace

void SurfacePool_SetLevelsCalls(const AddrCall *table, u32 count) { g_levelsCalls.Set(table, count); }

extern "C" u32 __fastcall SurfacePool_MakeLevels(u8 *pool, void *, s32 side, u32 levels, u32 format) {
    u32 rounds = (levels ? levels : 1) + 5;
    for (; rounds; --rounds) {
        if (side <= 0) return 1;
        if (!g_levelsCalls.Th(0x0058b0b0, address(pool), side, format)) {   // SurfacePool_MakeSurfaces
            g_levelsCalls.Th(0x0058b280, address(pool));
            return 0;
        }
        side >>= 1;
    }
    return 1;
}

extern "C" u32 __fastcall SurfacePool_ReleaseAll(u8 *pool) {
    for (u32 i = 0; i < 32; i++) {
        u8 *const surface = reinterpret_cast<u8 **>(pool)[i];
        if (!surface) continue;
        comcall<u32>(surface, 0x98, 0u);   // (its answer reaches no test, sic)
        const s32 released = (s32)comcall<u32>(surface, 8);
        if (released < 0) g_poolCalls.Cd(0x00564650, g_poolCalls.Cd(0x0056a770, (u32)released));
        reinterpret_cast<u8 **>(pool)[i] = nullptr;
    }
    return 1;
}

extern "C" u32 __fastcall SurfacePool_MakeSurfaces(u8 *pool, void *, s32 side, u32 format) {
    u8 *const device = at<u8 *>(global32p(0x00879a24), 8);
    const s32 index = SurfacePool_SideIndex(side);
    u8 desc[0x7c] = {};
    at<u32>(desc, 0) = 0x7c;
    at<u32>(desc, 0x68) = 0x1800;
    at<u32>(desc, 4) = 0x1007;
    at<u32>(desc, 0xc) = (u32)side;
    at<u32>(desc, 8) = (u32)side;
    const u32 *record;
    const u32 found = g_poolCalls.Th(0x00573820, global32(0x00879da8), &record, format);
    if (!found)
        return found;
    std::memcpy(desc + 0x48, record, 0x20);
    u8 **slot = reinterpret_cast<u8 **>(pool + index * 8);
    for (s32 i = 0; i < 2; ++i, ++slot) {
        u32 result = kept(comcall<u32>(device, 0x18, desc, slot, 0u));
        if (!result) {
            result = kept(comcall<u32>(*slot, 0x94, 0u));
            if (!result)
                continue;
        }
        g_poolCalls.Th(0x0056af20, 0x00879ad0u, 3u, result);
        return 0;
    }
    return 1;
}
