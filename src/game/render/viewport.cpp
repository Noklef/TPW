// The render view's screen and window geometry: reimplementation of 0x582eb0-0x5832dc.
#include "viewport.h"

#include <cstdint>

#include "../math/x87.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename T> T &at(void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

constexpr float kEdge = 0.999f;     // 0x701dc8
constexpr float kHalf = 0.5f;       // 0x701dcc
constexpr float kQuarter = -0.25f;  // 0x701dd0

// fild a; fild b; fdiv: the exact quotient of two dwords rounded once to a 24-bit significand (a double quotient would round twice).
double DivInt(s32 a, s32 b) {
    if (a == 0 || b == 0) return (double)a / (double)b;
    const unsigned long long ua = a < 0 ? 0ull - (unsigned long long)(long long)a : (unsigned long long)a;
    const unsigned long long ub = b < 0 ? 0ull - (unsigned long long)(long long)b : (unsigned long long)b;
    int bits = 0;
    while ((ua >> bits) != 0) ++bits;
    const int shift = 63 - bits;
    const unsigned long long num = ua << shift, q = num / ub, rem = num % ub;
    const double r = X87_Round24((q << 1) | (rem != 0 ? 1 : 0), -shift - 1);
    return (a < 0) != (b < 0) ? -r : r;
}

// __ftol: truncated to 64 bits, the low half.
s32 Ftol(X87Reg x) { return (s32)(long long)x.v; }

// The centre's offset by the pixel edge (0.999 * -0.25 taken off).
float Edge(X87Reg centre) { return (float)(centre - X87Reg(kEdge) * kQuarter); }

}  // namespace

extern "C" {

void __fastcall View_SetScreen(u8 *view, void *, s32 width, s32 height, u32 depth) {
    const float heightf = (float)height;
    const X87Reg aspect = DivInt(height, width);
    at<s32>(view, 0x08) = width;
    at<s32>(view, 0x0c) = height;
    at<u32>(view, 0x1c) = depth;
    at<float>(view, 0x04) = (float)aspect;
    at<float>(view, 0x10) = (float)((X87Reg((double)width) - kEdge) * kHalf);
    const float halfh = (float)((X87Reg(heightf) - kEdge) * kHalf);
    at<float>(view, 0x14) = halfh;
    const float halfw = at<float>(view, 0x10);
    at<s32>(view, 0x20) = width;
    at<float>(view, 0x38) = halfw;
    at<s32>(view, 0x24) = height;
    at<float>(view, 0x3c) = halfh;
    at<float>(view, 0x30) = halfw;
    at<float>(view, 0x34) = halfh;
    at<float>(view, 0x28) = Edge(halfw);
    at<float>(view, 0x2c) = Edge(halfh);
    if (global<u32>(0x008bd524) == 4) at<u32>(view, 0) &= 0xfffeffff;
}

void __fastcall View_SetWindow(u8 *view, void *, float x, float y, float width, float height) {
    const s32 screenw = at<s32>(view, 0x08);
    const s32 winw = Ftol(X87Reg(X87_MulIntDouble(screenw, width)) * kHalf);
    const float screenh = (float)at<s32>(view, 0x0c);
    const s32 winh = Ftol(X87Reg(height) * screenh * kHalf);
    at<s32>(view, 0x20) = winw;
    at<s32>(view, 0x24) = winh;
    const X87Reg left = X87Reg(x) - -1.0f;  // 0x701dd4
    const X87Reg span = X87_MulIntDouble(screenw, left.v);
    at<float>(view, 0x38) = (float)((X87Reg((double)winw) - kEdge) * kHalf);
    at<float>(view, 0x3c) = (float)((X87Reg((double)winh) - kEdge) * kHalf);
    const s32 offx = Ftol(span * -0.5f);  // 0x701dd8
    const X87Reg top = X87Reg(height) + y;
    const X87Reg centrex = X87Reg(at<float>(view, 0x38)) - (double)offx;
    const X87Reg down = X87_SubR(top, 1.0f) * screenh;
    at<float>(view, 0x30) = (float)centrex;
    const s32 offy = Ftol(down * -0.5f);
    const float centrey = (float)(X87Reg(at<float>(view, 0x3c)) - (double)offy);
    at<float>(view, 0x34) = centrey;
    at<float>(view, 0x28) = Edge(centrex);
    at<float>(view, 0x2c) = Edge(centrey);
}

void __fastcall View_ResetWindow(u8 *view) {
    const float halfw = X87_Through(at<float>(view, 0x10)), halfh = X87_Through(at<float>(view, 0x14));
    const s32 width = at<s32>(view, 0x08);
    at<float>(view, 0x38) = halfw;
    at<s32>(view, 0x20) = width;
    at<float>(view, 0x3c) = halfh;
    at<float>(view, 0x30) = halfw;
    at<s32>(view, 0x24) = at<s32>(view, 0x0c);
    at<float>(view, 0x34) = halfh;
    at<float>(view, 0x28) = Edge(halfw);
    at<float>(view, 0x2c) = Edge(halfh);
}

void __fastcall View_SetProjection(u8 *view, void *, u32, float a, float b) {
    const float s40 = at<float>(view, 0x40), s4c = at<float>(view, 0x4c), s50 = at<float>(view, 0x50), s54 = at<float>(view, 0x54);
    const X87Reg q = X87Reg(a) / b;
    const X87Reg d = (X87Reg(s50) - s54) / (q - 1.0f);  // 0x701ddc
    const X87Reg e = (X87Reg(s50) - s4c) / a;
    const X87Reg f = X87Reg(s54) - d;
    const X87Reg g = X87Reg(s40) * s4c;
    at<float>(view, 0x60) = X87_Through(a);
    at<float>(view, 0x6c) = (float)(e * s40);
    at<float>(view, 0x70) = (float)g;
    at<float>(view, 0x64) = (float)(d * s40);
    at<float>(view, 0x68) = (float)(f * s40);
}

void __fastcall View_SetDepthRange(u8 *view, void *, float near_, float far_) {
    const X87Reg r = X87Reg(1.0f) / (X87Reg(far_) - near_);
    const X87Reg nn = X87Reg(near_) * near_, ff = X87Reg(far_) * far_, rr = r * r;
    at<float>(view, 0x8c8) = X87_Through(near_);
    at<float>(view, 0x8cc) = X87_Through(far_);
    at<float>(view, 0x8d0) = (float)r;
    at<float>(view, 0x8d4) = (float)nn;
    at<float>(view, 0x8d8) = (float)ff;
    at<float>(view, 0x8dc) = (float)rr;
}
u8 *__fastcall View_Defaults(u8 *view) {
    static const u32 kDefaults[][2] = {
        {0x0, 0x410100}, {0x8, 0}, {0xc, 0}, {0x1c, 0}, {0x4, 0}, {0x10, 0}, {0x14, 0}, {0x60, 0}, {0x48, 0},
        {0x4c, 0x3dcccccd}, {0x50, 0x3e4ccccd}, {0x54, 0x3f800000}, {0x44, 0x3b449ba6}, {0x18, 0}, {0x78, 0}, {0x7c, 0},
        {0x40, 0x3f800000}, {0x58, 0x3dcccccd}, {0x5c, 0x3dcccccd},
        {0x8c8, 0x42c80000}, {0x8cc, 0x43020000}, {0x8d0, 0x3d088889}, {0x8d4, 0x461c4000}, {0x8d8, 0x46840800}, {0x8dc, 0x3a91a2b4},
        {0x8b4, 0x400080}, {0x8b8, 0}, {0x8bc, 0x4a800000}, {0x8c0, 0}, {0x8c4, 0x43000000}, {0x8b0, 0x3fc00000},
        {0x8e0, 0x3e99999a}, {0x8e4, 0x80}, {0x8e8, 0x20}, {0x8ec, 1}, {0x8f0, 0}, {0x8f4, 0x4f000000}, {0x8f8, 0x43c80000},
        {0x924, 0x3f000000},
    };
    for (const auto &d : kDefaults)
        at<u32>(view, d[0]) = d[1];
    return view;
}
}  // extern "C"
