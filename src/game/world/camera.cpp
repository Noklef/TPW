#include "camera.h"

#include <cmath>

#include "../core/gameclock.h"
#include "../math/mat4.h"
#include "../math/matrix.h"
#include "../math/x87.h"
#include "../render/viewquad.h"
#include "mapcell.h"
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

void __cdecl Camera_SnapToRide() {
    float &x = global<float>(0x007908f0);
    float &z = global<float>(0x007908f8);
    s32 cx = (s32)(long long)(X87Reg(x) * 0.100000001f).v;  // 0x6fdd58, __ftol
    s32 cz = (s32)(long long)(X87Reg(z) * 0.100000001f).v;
    if (cx < 0) cx = 0;
    if (cz < 0) cz = 0;
    if (cx >= global<s32>(0x007a0314)) cx = global<s32>(0x007a0314) - 1;
    if (cz >= global<s32>(0x007a0374)) cz = global<s32>(0x007a0374) - 1;
    const u8 *const thing = Cell_FindRideableObject(cx, cz);
    if (!thing) return;
    const u32 id = *reinterpret_cast<const u16 *>(thing + 0x38);
    const u32 v = id - 1;
    x = (float)X87_SubR(X87_MulInt(v & 0x7f, -10.0f), 5.0f);  // 0x6fdd5c, 0x6fdd60
    z = (float)X87_SubR(X87_MulInt(v >> 7, -10.0f), 5.0f);
    const u8 *const world = **reinterpret_cast<u8 *const *const *>(0x007cf6ec);
    u32 &yaw = global(0x00790a38);
    switch (MapCell_Direction(world + 0x294 + id * 0x44) & 0x55) {
    case 1: yaw = 0x40490fdb; break;   // pi
    case 4: yaw = 0x3fc90fdb; break;   // pi / 2
    case 0x10: yaw = 0; break;
    case 0x40: yaw = 0x4096cbe4; break;   // 3 pi / 2
    }
}

void __cdecl Camera_Reset() {
    global(0x007909ec) = global(0x006fdd40);
    global(0x00790a38) = global(0x006fdd44);
    global(0x00790ab0) = 0;
    global(0x007908f0) = global(0x006fdd48);
    global(0x007908f4) = global(0x006fdd4c);
    global(0x007908f8) = global(0x006fdd50);
    Matrix_Identity(&global<float>(0x00790990));
    Matrix_Identity(&global<float>(0x00790a48));
    Matrix_Identity(&global<float>(0x00790948));
    global(0x007909c4) = X87_FloatThrough(global(0x007908f4));
    global(0x007909c0) = global(0x007908f0);
    global(0x007909c8) = X87_FloatThrough(global(0x007908f8));
    Mat4_Assign(global<u32 *>(0x0087afe8), nullptr, &global(0x00790990));
    const float fov = (float)(X87Reg(global<float>(0x0074c9b8)) * X87Reg(global<float>(0x006fdd80)));
    Mat4_Perspective(reinterpret_cast<float *>(0x0087b110), nullptr, 0.1f, 1000.0f, fov);
    ViewQuad_SetFov(global<float *>(0x00879d54), nullptr, fov);
    const u32 flags = (global(0x00790aac) & 0x200) | 0x80;
    global(0x00790aac) = flags;
    global(0x007909d0) = flags;
    global(0x007909e4) = Clocks_RealNow(reinterpret_cast<u8 *>(0x00785970));
    global(0x00790a88) = 0;
    global(0x007908e8) = 1;
    global(0x00790a8c) = 0;
    global(0x00790a90) = 0;
    global(0x00790ab0) = 0;
}

void __cdecl Camera_SnapYaw() {
    constexpr float kHalfPi = 1.57079637f, kQuarterPi = 0.785398185f;   // (0x6fdd68 / 0x6fdd98; 0x6fdd88 is the half as a double)
    float &yaw = global<float>(0x00790a38);
    const X87Reg r = std::fmod((double)yaw, (double)kHalfPi);   // _CIfmod (0x67b24a), exact
    if (!(r.v >= 0.0)) {   // (below 0 or unordered)
        if (!(r.v >= -kQuarterPi))
            yaw = (float)X87_SubR(r - X87Reg(-kHalfPi), X87Reg(yaw));
        else
            yaw = (float)(X87Reg(yaw) - r);
    } else if (r.v > kQuarterPi) {
        yaw = (float)((X87Reg(kHalfPi) - r) + X87Reg(yaw));
    } else {
        yaw = (float)(X87Reg(yaw) - r);
    }
}

void __cdecl Camera_Input(s32 message, u32 param) {
    const s32 height = global<s16>(0x00faa5c4), width = global<s16>(0x00faa5c0);
    const s32 y = (s32)(param >> 16) * height / 2048;       // (idiv by shifts and a magic multiply: truncated)
    const s32 x = (s32)(param & 0xffff) * width / 0x600;
    u32 &flags = global(0x00790aac);
    const auto set_point = [x, y] {
        global<float>(0x00790ac8) = (float)y;
        global<float>(0x00790acc) = (float)x;
    };
    const auto by_button = [param, &flags](bool on) {   // (buttons 0, 1, 2: bits 1, 4, 2)
        const u32 bit = param == 0 ? 1 : param == 1 ? 4 : param == 2 ? 2 : 0;
        flags = on ? flags | bit : flags & ~bit;
    };
    switch (message) {
    case 0x1e:
        if (flags & 2) {
            float &yaw = global<float>(0x00790a38);
            yaw = (float)((((X87Reg(global<float>(0x00790ac8)) * 2048.0f) / X87Reg((double)height)) - 1024.0f) * global<float>(0x0074c9c0) + yaw);
        }
        break;
    case 0x20:
        flags &= 0xffffff87;
        break;
    case 0x22:
        flags &= 0xffffff80;
        global(0x00790ab0) |= 0x100;
        break;
    case 0x10001:
        global(0x00790ab0) &= ~0x100u;
        set_point();
        break;
    case 0x10002:
        global(0x00790ab0) |= 0x100;
        break;
    case 0x10003:
        set_point();
        break;
    case 0x10004:
        by_button(false);
        break;
    case 0x10005:
        by_button(true);
        break;
    case 0x1000d: {   // (the wheel: 0x7909ec less -10 for a negative param, less 10 otherwise)
        float &zoom = global<float>(0x007909ec);
        zoom = (float)(X87Reg(zoom) - ((s32)param < 0 ? -10.0f : 10.0f));
        break;
    }
    default:
        break;
    }
}
}
