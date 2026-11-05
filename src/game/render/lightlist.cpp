#include "lightlist.h"

#include <cstdint>

#include "../math/x87.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

// The C library's asin (0x67db20) and acos (0x67b4e0) of a register, as in math/mat4.cpp.
bool crt_arc_special(X87Reg x, double &result) {
    const unsigned long long bits = X87_Bits(x.v);
    const u32 exponent = static_cast<u32>(bits >> 52) & 0x7ff;
    if (exponent < 0x3ff) return false;
    if (x.v != x.v) result = x.v;
    else if (exponent == 0x3ff && (bits & 0xfffffffffffffULL) == 0) result = 2.0;
    else result = X87_FromBits(0xfff8000000000000ULL);
    return true;
}
double crt_asin(X87Reg x) {
    double special;
    if (crt_arc_special(x, special)) return special == 2.0 ? (x.v > 0 ? 1.5707963267948966 : -1.5707963267948966) : special;
    const X87Reg r = X87_Sqrt((X87Reg(1.0) + x) * (X87Reg(1.0) - x));
    return X87_Atan2(x.v, r.v);
}
double crt_acos(X87Reg x) {
    double special;
    if (crt_arc_special(x, special)) return special == 2.0 ? (x.v > 0 ? 0.0 : 3.141592653589793) : special;
    const X87Reg r = X87_Sqrt((X87Reg(1.0) + x) * (X87Reg(1.0) - x));
    return X87_Atan2(r.v, x.v);
}

// 1 / sqrt(squared); 1 when squared is 0 or a NaN (the FCOM's C3).
X87Reg inverse_length(X87Reg squared) {
    if (!(squared.v < 0 || squared.v > 0)) return X87Reg(1.0);
    return 1.0f / X87_Sqrt(squared);
}

}  // namespace

extern "C" {

void __fastcall LightList_Clear(u8 *list) {
    at<u32>(list, 0x1c4) = 0;
    at<u32>(list, 0x1c0) = 0;
}

u8 *__fastcall LightList_First(u8 *list) {
    at<u32>(list, 0x1c4) = 0;
    return at<s32>(list, 0x1c0) > 0 ? list + 0x40 : nullptr;
}

u8 *__fastcall LightList_Next(u8 *list) {
    u32 cursor = at<u32>(list, 0x1c4) + 1;  // wraps, as the original's INC
    at<u32>(list, 0x1c4) = cursor;
    if ((s32)cursor >= at<s32>(list, 0x1c0))
        return nullptr;
    return list + ((cursor + 1) << 6);
}

double __cdecl LightList_Angle(float a1, float a2, float a3, float a4) {
    const X87Reg dot = X87Reg(a3) * a1 + X87Reg(a4) * a2;
    const X87Reg length_b = X87_Sqrt(X87Reg(a3) * a3 + X87Reg(a4) * a4);
    const X87Reg length_a = X87_Sqrt(X87Reg(a1) * a1 + X87Reg(a2) * a2);
    const X87Reg length = length_b * length_a;
    const X87Reg cross = X87Reg(a4) * a1 - X87Reg(a3) * a2;
    const double side = crt_asin(cross / length);
    const double turn = crt_acos(dot / length);
    if (side >= 0.0) return turn;
    return (X87Reg(3.14159274101257324) + X87Reg(3.14159274101257324) - X87Reg(turn)).v;
}

u32 __fastcall LightList_Add(u8 *list, void *, float x, float y, float z) {
    const s32 count = at<s32>(list, 0x1c0);
    if (count >= 4) return 0;
    const u32 i = (u32)count;
    const X87Reg scale = inverse_length(X87Reg(x) * x + X87Reg(y) * y + X87Reg(z) * z);
    x = (float)(scale * x);
    y = (float)(scale * y);
    z = (float)(scale * z);
    at<float>(list, 0x170 + i * 12) = x;
    at<float>(list, 0x174 + i * 12) = y;
    at<float>(list, 0x178 + i * 12) = z;

    u8 *const entry = list + ((i + 1) << 6);
    for (u32 k = 0; k < 0x10; ++k) at<u32>(entry, k * 4) = 0;
    at<float>(entry, 0) = 1.0f;
    at<float>(entry, 0x28) = 1.0f;
    at<float>(entry, 0x3c) = 1.0f;
    at<float>(entry, 0x10) = (float)-(X87Reg(x) / y).v;
    at<float>(entry, 0x18) = (float)-(X87Reg(z) / y).v;

    const float gx = global<float>(0x00768ab0), gy = global<float>(0x00768ab4), gz = global<float>(0x00768ab8);
    at<float>(list, 0x140 + i * 12) = (float)(X87Reg(z) * gy - X87Reg(y) * gz);
    at<float>(list, 0x144 + i * 12) = (float)(X87Reg(x) * gz - X87Reg(z) * gx);
    at<float>(list, 0x148 + i * 12) = (float)(X87Reg(y) * gx - X87Reg(x) * gy);

    const X87Reg lit = (X87Reg(x) * gx + X87Reg(y) * gy + X87Reg(z) * gz) * -127.0f;
    if (!(lit.v > 0)) return 1;
    at<u32>(list, 0x1b0 + i * 4) = (u32)(s32)(long long)lit.v << 24;   // (__ftol 0x67a830: truncated through a qword)

    const X87Reg flat = inverse_length(X87Reg(x) * x + X87Reg(y) * y);
    const float flat_x = (float)(flat * x), flat_z = (float)(flat * z);
    at<float>(list, 0x1a0 + i * 4) = (float)LightList_Angle(0.0f, 1.0f, flat_x, flat_z);
    at<s32>(list, 0x1c0) = count + 1;
    return 1;
}
}
