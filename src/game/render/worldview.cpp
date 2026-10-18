#include "worldview.h"

#include <cstdint>

#include "../math/x87.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

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
}  // extern "C"
