#include "worldview.h"

#include <cstdint>

#include "../math/mat4.h"
#include "../math/x87.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

// A ray parameter as a float, held to lim when below 0 (or a NaN: the x87's compare with 0) or above lim.
float Held(X87Reg t, float zero, X87Reg lim) {
    const float f = (float)t;
    return !(t.v >= zero) || f > lim.v ? (float)lim : f;
}

// fld end; fsub start; fdiv step; __ftol: the count of steps (low half of the 64-bit truncation).
s32 steps(float start, float end, float step) {
    const X87Reg n = (X87Reg(end) - X87Reg(start)) / X87Reg(step);
    return (s32)(u32)(long long)n.v;
}

// fcomp 0.0 sets C3 for equal or unordered: such a step leaves the count as it was.
bool counts(float step) { return step == step && step != 0.0f; }

}  // namespace

extern "C" {

void __fastcall WorldView_UpdateDiagonal(u8 *view) {
    const X87Reg dx = X87Reg(at<float>(view, 0x40)) - X87Reg(at<float>(view, 0x3c));
    const X87Reg dy = X87Reg(at<float>(view, 0x48)) - X87Reg(at<float>(view, 0x44));
    const float stored = (float)dx;  // fst [esp]
    at<float>(view, 0x38) = (float)X87_Sqrt(dx * X87Reg(stored) + dy * dy);
}

void __fastcall WorldView_SetRangeX(u8 *view, void *, float start, float end, float step) {
    at<float>(view, 0x3c) = X87_Through(start);
    at<float>(view, 0x40) = X87_Through(end);
    at<float>(view, 0x4c) = X87_Through(step);
    if (counts(step)) {
        const s32 n = steps(start, end, step);
        at<s32>(view, 0x54) = n;
        at<s32>(view, 0x5c) = n - 1;
    }
}

void __fastcall WorldView_SetRangeY(u8 *view, void *, float start, float end, float step) {
    at<float>(view, 0x44) = X87_Through(start);
    at<float>(view, 0x48) = X87_Through(end);
    at<float>(view, 0x50) = X87_Through(step);
    for (int pass = 0; pass < 2; ++pass) {  // the same test and count twice over (sic)
        if (counts(step)) {
            const s32 n = steps(start, end, step);
            at<s32>(view, 0x58) = n;
            at<s32>(view, 0x60) = n - 1;
        }
    }
}

u32 __fastcall Obj_Set30_34(u8 *object, void *, u32 a, u32 b) {
    at<u32>(object, 0x30) = a;
    at<u32>(object, 0x34) = b;
    return a;
}
float *__fastcall WorldView_RayPoint(const u8 *view, void *, float *out, const float *vector) {
    const float *const m = global<const float *>(0x0087afe8);
    float d[3];
    Mat4_Transform(m, nullptr, d, vector, 3);
    const float zero = global<float>(0x007014a0);
    if (!(d[1] < zero || d[1] > zero))   // (equal or unordered)
        d[1] = global<float>(0x00701488);
    const X87Reg lim = X87Reg(global<float>(0x008bd4c0)) * global<float>(0x0070149c);
    const float first = Held(X87_SubR(at<float>(m, 0x34), at<float>(view, 0x30)) / d[1], zero, lim);
    const float second = Held(X87_SubR(at<float>(m, 0x34), at<float>(view, 0x34)) / d[1], zero, lim);
    X87Reg s = second;
    if (first < second || first > second) {
        s = (X87Reg(second) + first) * global<float>(0x0070149c);
        const u8 *const grid = global<const u8 *>(0x00879d60);
        const u8 *const origin = *reinterpret_cast<const u8 *const volatile *>(grid);
        (void)*reinterpret_cast<const volatile u32 *>(origin + 8);
        (void)*reinterpret_cast<const volatile u32 *>(grid + 0x10);
        (void)*reinterpret_cast<const volatile u32 *>(origin + 0xc);
        (void)*reinterpret_cast<const volatile u32 *>(grid + 0x14);
    }
    out[0] = (float)(s * d[0] + at<float>(m, 0x30));
    out[1] = (float)(s * d[1] + at<float>(m, 0x34));
    out[2] = (float)(s * d[2] + at<float>(m, 0x38));
    return out;
}
}  // extern "C"
