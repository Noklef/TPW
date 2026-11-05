#include "weathertables.h"

#include <cstdint>

#include "../core/crt.h"
#include "../core/gameheap.h"
#include "../math/x87.h"

namespace {

AddrCalls g_calls;

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

float sub(float a, float b) { return (float)(X87Reg(a) - b); }

// 0..0x3fff less the constant at 0x701c6c, times the one at 0x701c70.
float cubeCoord() { return (float)((X87Reg((double)(Crt().rand() & 0x3fff)) - global<float>(0x00701c6c)) * global<float>(0x00701c70)); }

// -0x400..0x3ff times the constant at 0x701c90.
float direction() { return (float)(X87Reg((double)((Crt().rand() & 0x7ff) - 0x400)) * global<float>(0x00701c90)); }

s32 dropRandom() { return Crt().rand() % 0x126eb + 1; }

}  // namespace

void WeatherTables_SetCalls(const AddrCall *table, u32 count) { g_calls.Set(table, count); }

u32 __cdecl WeatherFx_Create() {
    if (global<u32>(0x008bcbac)) return 0;
    void *const block = GameHeap_Alloc(0xa1cc);
    const u32 made = block ? g_calls.Th(0x0057d1d0, (u32)(uintptr_t)block) : 0;
    global<u32>(0x008bcbac) = made;
    if (!made) {
        g_calls.Th(0x0056af20, 0x00879ad0u, 3u, 0xb0754000u);
        return 0;
    }
    WeatherFx_FillTables(reinterpret_cast<u8 *>((uintptr_t)made));
    return 1;
}

u32 __fastcall WeatherFx_FillTables(u8 *fx) {
    for (u32 i = 0; i < 0x400 * 3; ++i) at<float>(fx, 8 + 4 * i) = cubeCoord();
    for (s32 i = 0; i < 0x80; ++i) {
        const X87Reg n((double)i);
        const X87Reg angle = X87Reg(global<float>(0x00701c10)) * n * global<float>(0x00701c74);
        const X87Reg fade = global<float>(0x00701c80) - n * global<float>(0x00701c74);
        const X87Reg wave = X87Reg(X87_Cos(angle.v)) - X87Reg(global<double>(0x00701c78));
        at<float>(fx, 0x3358 + 4 * i) = (float)(wave * fade * global<float>(0x00701c84));
    }
    for (s32 i = 0; i < 0x400; ++i) {
        const X87Reg start = X87Reg((double)dropRandom()) * global<float>(0x00701c88);
        at<float>(fx, 0x4558 + 4 * i) = (float)X87_DivR(start - global<float>(0x00701c8c), global<float>(0x00701c68));
        for (u32 c = 0; c < 3; ++c) at<float>(fx, 0x5558 + 12 * i + 4 * c) = direction();
        at<float>(fx, 0x3558 + 4 * i) = (float)((float)i - X87Reg((double)dropRandom()) * global<float>(0x00701c94));
    }
    float ring[0x80][3];
    for (auto &point : ring)
        for (float &c : point) c = direction();
    for (u32 i = 0; i < 0x80; ++i) {
        const float *const p0 = ring[(i - 3) & 0x7f], *const p1 = ring[(i - 2) & 0x7f], *const p2 = ring[(i - 1) & 0x7f], *const p3 = ring[i];
        u8 *const record = fx + 0x8558 + 0x30 * i;
        for (u32 c = 0; c < 3; ++c) {
            const float d = sub(p0[c], p1[c]), g = sub(sub(p3[c], p2[c]), d);
            at<float>(record, 4 * c) = g;
            at<float>(record, 0xc + 4 * c) = sub(d, g);
            at<float>(record, 0x18 + 4 * c) = sub(p2[c], p0[c]);
            at<float>(record, 0x24 + 4 * c) = p1[c];
        }
    }
    at<u32>(fx, 0x3344) = global<u32(__stdcall *)()>(0x006fd1d0)();
    return 1;
}
