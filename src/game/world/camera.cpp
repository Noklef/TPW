#include "camera.h"

#include <cmath>

#include "../math/x87.h"
#include "things.h"

namespace {

template <typename T = u32> T &global(u32 address) { return *reinterpret_cast<T *>(address); }

}  // namespace

extern "C" {

void __cdecl Camera_InitUnusedOne() { global<float>(0x007909e8) = 1.0f; }

void __cdecl Camera_InitUnusedEpsilon() { global<float>(0x00790a94) = 1.0f / 8388608.0f; }

void __cdecl Camera_Init7909ec() { global<float>(0x007909ec) = 110.0f; }

void __cdecl Camera_InitYaw() { global<float>(0x00790a38) = 0.0f; }

void __cdecl Camera_Init7909e0() { global<float>(0x007909e0) = 0.0f; }

void __cdecl Camera_InitPoint() {
    global<float>(0x007908f0) = 475.0f;
    global<float>(0x007908f4) = 0.0f;
    global<float>(0x007908f8) = 175.0f;
}

u32 __cdecl Camera_IsFollowing(const void *thing) {
    const void *const followed = global<const void *>(0x00790a44);
    return (global<u8>(0x00790ab0) & 4) && global(0x00790988) && followed && followed == thing;
}

u32 __cdecl Camera_MatricesClose(const float *a, const float *b) {
    const auto diff = [a, b](u32 i) { return X87Reg(std::fabs((X87Reg(a[i]) - b[i]).v)); };
    // (|d1| + |d0|), then for each pair (j, k): (|dj| + stored) + |dk|.
    X87Reg sum = diff(1) + diff(0);
    const u32 pairs[5][2] = {{2, 4}, {5, 6}, {8, 9}, {10, 12}, {13, 14}};
    for (const auto &pair : pairs) sum = diff(pair[0]) + (float)sum + diff(pair[1]);
    return !(sum.v >= global<float>(0x0074c980));  // fcomp: NaN counts as less
}

const void *__cdecl Camera_Followed() {
    return (global<u8>(0x00790ab0) & 4) && global(0x00790988) ? global<const void *>(0x00790a44) : nullptr;
}

u8 *__cdecl Camera_PartFace(u8 *const *instance, const u32 *part) {
    const u8 *const model = *reinterpret_cast<u8 *const *>(*instance + 4);
    const u32 index = *reinterpret_cast<const u16 *>(model + 0x46) + *part;
    const u32 first = *reinterpret_cast<const u16 *>(model + 0x44);
    return index < first ? *reinterpret_cast<u8 *const *>(model + 0x70) + index * 0xa0
                         : *reinterpret_cast<u8 *const *>(model + 0x74) + (index - first) * 0x58;
}

void __cdecl Camera_SetView(const u32 *x, const u32 *z, const u32 *zoom, const u32 *yaw, const u32 *other) {
    global(0x00790a88) = 0;
    global(0x00790a8c) = 0;
    global(0x00790a90) = 0;
    if (x) {
        global(0x007908f0) = *x;
        global(0x007908f8) = *z;  // sic: tests x again, so z is read whenever x is given
    }
    if (zoom) global(0x007909ec) = *zoom;
    if (yaw) global(0x00790a38) = *yaw;
    if (other) global(0x0074c9bc) = *other;
}

void __cdecl Camera_CentreOnCell(const u16 *cell) {
    const u32 v = static_cast<u32>(*cell) - 1;
    const float x = static_cast<float>(static_cast<s32>((v & 0x7f) * 10));
    const float z = static_cast<float>(static_cast<s32>((v >> 7) * 10));
    Camera_SetView(reinterpret_cast<const u32 *>(&x), reinterpret_cast<const u32 *>(&z), nullptr, nullptr, nullptr);
}

void __cdecl Camera_CentreOnThing(u32 handle) {
    const u16 h = static_cast<u16>(handle);
    if (!h) return;
    const u8 *const thing = *reinterpret_cast<const u8 *const *>(0x007cfb90 + 20 * static_cast<u32>(h));
    if (!thing) return;
    const u16 cell = static_cast<u16>(thing[7] * 128 + thing[5] + 1);
    Camera_CentreOnCell(&cell);
}

void __cdecl Camera_SetFlag80(u32 on) {
    if (on)
        global(0x00790ab0) |= 0x80;
    else
        global(0x00790ab0) &= ~0x80u;
}

void __cdecl Camera_ClampCell(s32 *x, s32 *z) {
    if (*x < 0) *x = 0;
    if (*z < 0) *z = 0;
    if (*x >= global<s32>(0x007a0314)) *x = global<s32>(0x007a0314) - 1;
    if (*z >= global<s32>(0x007a0374)) *z = global<s32>(0x007a0374) - 1;
}

void __cdecl Camera_CellOf(const float *position, s32 *x, s32 *z) {
    if (x) *x = (s32)(long long)(X87Reg(position[0]) * 0.100000001f).v;  // 0x6fdd58, __ftol
    if (z) *z = (s32)(long long)(X87Reg(position[2]) * 0.100000001f).v;
}

void __cdecl Camera_PackCellFractions(u32 x, u32 z, u8 *outX, u8 *outZ) {
    const auto pack = [](u32 bits, u8 *out) {
        const X87Reg tenths = X87Reg(reinterpret_cast<const float &>(bits)) * 0.100000001f;  // 0x700184
        const s32 whole = (s32)(long long)tenths.v;  // __ftol
        out[1] = (u8)whole;
        out[0] = (u8)(s32)(long long)((tenths - X87Reg((double)(whole & 0xff))) * 256.0f).v;  // 0x700188
    };
    pack(x, outX);
    pack(z, outZ);
}

void __cdecl Camera_CentreOnGlobalThing() {
    const u8 *const thing = global<const u8 *>(0x007c2658);
    u16 handle;
    Thing_GetHandle(thing, nullptr, &handle);
    // (the high word what ECX held: the thing pointer's)
    Camera_CentreOnThing((static_cast<u32>(reinterpret_cast<uintptr_t>(thing)) & 0xffff0000u) | handle);
}
}
