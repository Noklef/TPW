// All coasters updated for a frame: see coastersupdate.h.
#include "coastersupdate.h"

#include <cmath>
#include <cstdint>
#include <cstring>

#include "../math/x87.h"

namespace {

AddrCalls g_calls;

template <typename T>
T &at(const void *p, u32 offset) {
    return *reinterpret_cast<T *>((uintptr_t)p + offset);
}
template <typename T>
T &global(u32 address) {
    return *reinterpret_cast<T *>((uintptr_t)address);
}
template <typename T>
T *pointer(u32 address) {
    return reinterpret_cast<T *>((uintptr_t)address);
}

const float kPerTick = 0.00100000005f, kOne = 1.0f;   // 0x6fdee8, 0x6fde70

// The texture offsets advanced; the step that is left for the V's.
float AdvanceOffsets(const u8 *coaster, u32 now) {
    const u32 elapsed = now - at<u32>(coaster, 0x138);
    float step = (float)(X87Reg((double)elapsed) * at<float>(coaster, 0x2c) * kPerTick);
    float *offset = at<float *>(coaster, 0x13c);
    for (u32 left = at<u32>(at<const u8 *>(coaster, 4), 0x20); left; --left, ++offset) {
        const X87Reg value = X87Reg(*offset) + step;
        *offset = (float)value;
        if (value.v > (double)kOne) {
            double whole;
            std::modf(value.v, &whole);   // CRT modf (0x67b280)
            const float wholeStored = (float)whole;
            *offset = (float)(X87Reg(*offset) - wholeStored);
            step = (float)(X87Reg(step) - wholeStored);
        }
    }
    return step;
}

// The V of each vertex of the shape used by a material with flag 0x20.
void ScrollShape(const u8 *shape, const u8 *materials, float step) {
    u32 used[0x100];
    memset(used, 0, sizeof(used));
    u32 highest = 0;
    const u8 *group = at<const u8 *>(shape, 0x6c);
    for (u32 left = at<u16>(shape, 0x5a); left; --left, group += 0x10) {
        const s32 index = (s32)(at<u32>(group, 0) - at<u32>(materials, 0x18)) >> 3;
        if (!(at<u8>(at<const u8 *>(materials, 0x10), (u32)(index << 4)) & 0x20)) continue;
        const u8 *face = at<const u8 *>(shape, 0x70) + at<u16>(group, 4) * 8u;
        for (u32 faces = at<u16>(group, 6); faces; --faces, face += 8) {
            const u32 a = at<u16>(face, 2), b = at<u16>(face, 4), c = at<u16>(face, 6);
            used[a] = used[b] = used[c] = 1;
            if (a > highest) highest = a;
            if (b > highest) highest = b;
            if (c > highest) highest = c;
        }
    }
    for (u32 i = 0; i <= highest; ++i) {
        if (!used[i]) continue;
        float *const block = reinterpret_cast<float *>(at<u8 *>(shape, 0x68) + (i >> 2) * 0x20);
        const u32 k = i & 3;
        const float scrolled = (float)(X87Reg(block[k + 4]) - step);
        block[k] = X87_Through(block[k]);
        block[k + 4] = scrolled;
    }
}

}  // namespace

void CoastersUpdate_SetCalls(const AddrCall *table, u32 count) { g_calls.Set(table, count); }

extern "C" {

void __cdecl Coasters_Update() {
    for (u8 *coaster = global<u8 *>(0x00790fe0); coaster; coaster = at<u8 *>(coaster, 0x18)) {
        if (!(at<u8>(coaster, 0) & 1)) continue;
        global<u8 *>(0x00792218) = coaster;
        global<u8 *>(0x00790fe4) = coaster + 0x3c;
        if (coaster == global<u8 *>(0x00790fec) && (at<u8>(coaster, 0) & 4)) g_calls.Cd(0x00435d50, coaster);   // Coaster_PlacePendingPiece
        if (!(at<u32>(coaster, 0) & 0x2000)) g_calls.Th(0x00403d00, 0x00785a08u, 0x43u, (u32)at<u16>(coaster, 0x14));   // ActionRec_WriteW
        g_calls.Cd(0x0043eb70, global<u32>(0x00792218));                                                               // Coaster_Rebuild
        at<u32>(coaster, 0) |= 0x2000;
    }
    global<u32>(0x00792218) = 0;
    for (u8 *coaster = global<u8 *>(0x00790fe0); coaster; coaster = at<u8 *>(coaster, 0x18)) {
        if (!(at<u8>(coaster, 8) & 0xe)) continue;
        g_calls.Cd(0x0043ce20, coaster);   // Coaster_RunTrains
        if (at<u8>(at<const u8 *>(coaster, 4), 0) & 4) {
            const u32 now = g_calls.Cd(0x004709e0, 0u);   // Motion_Time
            const float step = AdvanceOffsets(coaster, now);
            const u8 *const materials = at<const u8 *>(at<const u8 *>(coaster, 4), 0x1c);
            for (const u8 *car = at<const u8 *>(coaster, 0x38); car; car = at<const u8 *>(car, 0x38))
                ScrollShape(at<const u8 *>(at<const u8 *>(at<const u8 *>(car, 0x24), 4), 0x78), materials, step);
            at<u32>(coaster, 0x138) = now;
        }
        const u32 flags = at<u32>(coaster, 8);
        const bool stations = (flags & 0xc) && !(flags & 0x40);
        u8 *car = at<u8 *>(coaster, 0x128);
        for (u32 left = at<u32>(coaster, 0xf4); left; --left, car += 0x80) {
            if (!(at<u8>(car, 0) & 2)) continue;
            if (stations) g_calls.Cd(0x0043b8b0, coaster, car);   // CoasterTrain_AtStation
            g_calls.Cd(0x00439570, coaster, car);                 // CoasterCar_UpdateSounds
        }
    }
    global<u32>(0x00792220) = 0;
}
}
