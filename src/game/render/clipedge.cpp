#include "clipedge.h"

#include <cstdint>
#include <xmmintrin.h>

#include "../math/x87.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) {
    return *reinterpret_cast<T *>((uintptr_t)p + offset);
}

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

enum Plane { kYW, kYNegW, kXW, kXNegW, kZW, kZ0 };

X87Reg distance(Plane plane, const u8 *v) {
    const float x = at<float>(v, 0), y = at<float>(v, 4), z = at<float>(v, 8), w = at<float>(v, 0xc);
    switch (plane) {
    case kYW: return X87Reg(w) - y;
    case kYNegW: return X87Reg(y) + w;
    case kXW: return X87Reg(w) - x;
    case kXNegW: return X87Reg(x) + w;
    case kZW: return X87Reg(w) - z;
    default: return X87Reg(z);
    }
}

// fld a; fsub b; fmul t (the double at 0x879a18); fadd b
float lerp(const u8 *a, const u8 *b, u32 offset) {
    const float fb = at<float>(b, offset);
    return (float)((X87Reg(at<float>(a, offset)) - fb) * global<double>(0x00879a18) + fb);
}

void colour(u8 *out, const u8 *a, const u8 *b, u32 offset) {
    if (at<u32>(a, offset) == at<u32>(b, offset)) {
        at<u32>(out, offset) = at<u32>(b, offset);
        return;
    }
    // t * 255 made a float (in the argument a's slot, sic), then fistp: round to nearest, 0x80000000 out of range
    const float scaled = (float)(X87Reg(global<double>(0x00879a18)) * 255.0f);  // 0x70112c
    const s32 n = _mm_cvtss_si32(_mm_set_ss(scaled));
    global<s32>(0x00fb34a4) = n;
    global<s32>(0x00879968) = n;
    const u32 lo = at<u32>(b, offset) & 0xff00ff, hi = at<u32>(a, offset) & 0xff00ff;
    global<u32>(0x00879960) = lo;
    global<u32>(0x00879964) = hi;
    global<u32>(0x0087996c) = ((u32)((s32)((hi - lo) * (u32)n) >> 8) + lo) & 0xff00ff;
    const u32 b1 = at<u8>(b, offset + 1), a1 = at<u8>(a, offset + 1);
    global<u32>(0x00879960) = b1;
    global<u32>(0x00879964) = a1;
    const u32 g = (a1 - b1) * (u32)global<s32>(0x00fb34a4) + (b1 << 8);
    at<u32>(out, offset) = ((g | (at<u32>(a, offset) & 0xffff00ff)) & 0xff00ff00) | global<u32>(0x0087996c);  // (byte 3 or'd, sic)
}

void clip(Plane plane, u8 *out, const u8 *a, const u8 *b, s32 stride) {
    global<double>(0x008799f8) = distance(plane, a).v;
    global<double>(0x00879a00) = distance(plane, b).v;
    const double da = global<double>(0x008799f8), db = global<double>(0x00879a00);
    global<double>(0x00879a18) = ((X87Reg(0.9999f) * db) / (X87Reg(db) - da)).v;  // 0x701118
    at<float>(out, 0xc) = lerp(a, b, 0xc);
    u32 &left = global<u32>(0x00879a0c), &done = global<u32>(0x00879a08);
    left = (u32)stride - 0x18;
    done = 0;
    if (left != 0) {
        do {
            at<float>(out, done * 8 + 0x18) = lerp(a, b, done * 8 + 0x18);
            at<float>(out, done * 8 + 0x1c) = lerp(a, b, done * 8 + 0x1c);
            left -= 8;
            done += 1;
        } while (left != 0);
    }
    colour(out, a, b, 0x10);
    colour(out, a, b, 0x14);
    const u32 w = at<u32>(out, 0xc);
    switch (plane) {
    case kYW:
        at<float>(out, 0) = lerp(a, b, 0);
        at<u32>(out, 4) = w;
        at<float>(out, 8) = lerp(a, b, 8);
        break;
    case kYNegW:
        at<float>(out, 0) = lerp(a, b, 0);
        at<float>(out, 4) = -at<float>(out, 0xc);  // fld; fchs; fstp
        at<float>(out, 8) = lerp(a, b, 8);
        break;
    case kXW:
        at<u32>(out, 0) = w;
        at<float>(out, 4) = lerp(a, b, 4);
        at<float>(out, 8) = lerp(a, b, 8);
        break;
    case kXNegW:
        at<float>(out, 0) = -at<float>(out, 0xc);
        at<float>(out, 4) = lerp(a, b, 4);
        at<float>(out, 8) = lerp(a, b, 8);
        break;
    case kZW:
        at<float>(out, 0) = lerp(a, b, 0);
        at<float>(out, 4) = lerp(a, b, 4);
        at<u32>(out, 8) = w;
        break;
    default:
        at<float>(out, 0) = lerp(a, b, 0);
        at<float>(out, 4) = lerp(a, b, 4);
        at<u32>(out, 8) = 0;
        break;
    }
}

}  // namespace

extern "C" {

void __cdecl ClipEdge_YW(u8 *out, const u8 *a, const u8 *b, s32 stride) { clip(kYW, out, a, b, stride); }
void __cdecl ClipEdge_YNegW(u8 *out, const u8 *a, const u8 *b, s32 stride) { clip(kYNegW, out, a, b, stride); }
void __cdecl ClipEdge_XW(u8 *out, const u8 *a, const u8 *b, s32 stride) { clip(kXW, out, a, b, stride); }
void __cdecl ClipEdge_XNegW(u8 *out, const u8 *a, const u8 *b, s32 stride) { clip(kXNegW, out, a, b, stride); }
void __cdecl ClipEdge_ZW(u8 *out, const u8 *a, const u8 *b, s32 stride) { clip(kZW, out, a, b, stride); }
void __cdecl ClipEdge_Z0(u8 *out, const u8 *a, const u8 *b, s32 stride) { clip(kZ0, out, a, b, stride); }
}
