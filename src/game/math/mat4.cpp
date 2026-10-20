#include "mat4.h"

#include <cmath>
#include <cstdint>
#include <cstring>
#include <initializer_list>

#include "x87.h"
#include "../render/viewport.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

// a / b rounded once to a 24-bit significand, for an a of up to 53 bits (fsin's result): the double quotient can round onto a 24-bit
// midpoint, and its exact remainder (fma) says which side of it the true quotient is on.
double DivRound(double a, double b) {
    if (a != a || b != b) return (X87Reg(a) / X87Reg(b)).v;
    const double q = a / b;
    if (!std::isfinite(q) || q == 0) return q;
    int e;
    const double frac = std::frexp(std::fabs(q), &e);
    const unsigned long long m = (unsigned long long)std::ldexp(frac, 53);
    if ((m & ((1ull << 29) - 1)) != (1ull << 28)) return X87_Round(q);
    const double rem = std::fma(-q, b, a);
    if (rem == 0) return X87_Round(q);
    const bool above = (rem > 0) == (b > 0);  // a / b beyond q (towards +infinity)
    const double r = std::ldexp((double)((m >> 29) + (above == (q > 0) ? 1 : 0)), e - 24);
    return q < 0 ? -r : r;
}

}  // namespace

extern "C" {

void __cdecl Mat4_InitGlobals() {
    for (u32 address : {0x87b090u, 0x87b110u, 0x87aff0u, 0x87b030u, 0x87af68u, 0x87afa8u, 0x87b158u, 0x87b198u})
        Mat4_Identity(reinterpret_cast<u32 *>((uintptr_t)address));
}

void __fastcall Mat4_Identity(u32 *m) {
    for (int i = 0; i < 16; ++i)
        m[i] = 0;
    m[15] = m[10] = m[5] = m[0] = 0x3f800000;
}

void __fastcall Mat4_Assign(u32 *m, void *, const u32 *from) {
    for (int i = 0; i < 16; ++i)
        m[i] = from[i];
}

void __fastcall Mat4_MoveAlongX(float *m, void *, float distance) {
    const X87Reg x = X87Reg(distance) * m[0] + m[12];
    const X87Reg y = X87Reg(m[1]) * distance + m[13];
    const X87Reg z = X87Reg(m[2]) * distance + m[14];
    const X87Reg w = X87Reg(m[3]) * distance + m[15];
    m[12] = (float)x;
    m[13] = (float)y;
    m[14] = (float)z;
    m[15] = (float)w;
}

void __cdecl Mat4_LuBackSubstitute(const float *lu, const s32 *index, float *b) {
    u32 *bits = reinterpret_cast<u32 *>(b);
    s32 first = -1;
    for (s32 i = 0; i < 4; ++i) {
        const s32 ip = index[i];
        const float picked = b[ip];
        bits[ip] = bits[i];
        if (first >= 0) {
            X87Reg sum = picked;
            for (s32 j = first; j <= i - 1; ++j)
                sum = sum - X87Reg(b[j]) * lu[i * 4 + j];
            b[i] = (float)sum;
        } else {
            if (picked != 0 && picked == picked) first = i;
            b[i] = X87_Through(picked);
        }
    }
    for (s32 i = 3; i >= 0; --i) {
        X87Reg sum = b[i];
        for (s32 j = i + 1; j < 4; ++j)
            sum = sum - X87Reg(lu[i * 4 + j]) * b[j];
        b[i] = (float)(sum / lu[i * 5]);
    }
}

void __cdecl Mat4_LuDecompose(float *a, s32 *index, float *sign) {
    u32 *const bits = reinterpret_cast<u32 *>(a);
    float scale[4];
    *sign = 1.0f;
    for (int i = 0; i < 4; ++i) {
        float big = 0.0f;
        for (int j = 0; j < 4; ++j) {
            const float t = std::fabs(a[i * 4 + j]);
            if (t > big) big = t;   // (a NaN never)
        }
        scale[i] = (float)(X87Reg(1.0f) / big);
    }
    s32 pivot = 0;   // (kept from the column before; see the header for the first)
    for (int j = 0; j < 4; ++j) {
        for (int i = 0; i < j; ++i) {
            X87Reg sum = a[i * 4 + j];
            for (int k = 0; k < i; ++k)
                sum = sum - X87Reg(a[k * 4 + j]) * a[i * 4 + k];
            a[i * 4 + j] = (float)sum;
        }
        float big = 0.0f;
        for (int i = j; i < 4; ++i) {
            X87Reg sum = a[i * 4 + j];
            for (int k = 0; k < j; ++k)
                sum = sum - X87Reg(a[k * 4 + j]) * a[i * 4 + k];
            const X87Reg weight = X87Reg(std::fabs(sum.v)) * scale[i];
            a[i * 4 + j] = (float)sum;
            if (weight.v >= big) {   // (a NaN never)
                big = (float)weight;
                pivot = i;
            }
        }
        if (j != pivot) {
            for (int k = 0; k < 4; ++k) {
                const float t = a[pivot * 4 + k];
                bits[pivot * 4 + k] = bits[j * 4 + k];
                a[j * 4 + k] = X87_Through(t);
            }
            *sign = -*sign;
            scale[pivot] = scale[j];
        }
        index[j] = pivot;
        if (!(a[j * 5] < 0) && !(a[j * 5] > 0))   // (zero or a NaN)
            bits[j * 5] = 0x1e3ce508;   // 1e-20
        if (j != 3) {
            const float inverse = (float)(X87Reg(1.0f) / a[j * 5]);
            for (int i = j + 1; i < 4; ++i)
                a[i * 4 + j] = (float)(X87Reg(inverse) * a[i * 4 + j]);
        }
    }
}

void __fastcall Mat4_Invert(u32 *m) {
    u32 a[16], y[16];
    s32 index[4];
    float sign;
    std::memcpy(a, m, sizeof a);
    Mat4_LuDecompose(reinterpret_cast<float *>(a), index, &sign);
    for (int i = 0; i < 4; ++i) {
        u32 column[4] = {0, 0, 0, 0};
        column[i] = 0x3f800000;
        Mat4_LuBackSubstitute(reinterpret_cast<const float *>(a), index, reinterpret_cast<float *>(column));
        for (int j = 0; j < 4; ++j)
            y[j * 4 + i] = column[j];
    }
    std::memcpy(m, y, sizeof y);
}

void __fastcall Mat4_Perspective(float *m, void *, float near_, float far_, float fov) {
    const X87Reg angle = X87Reg(fov) * 0.0174532924f;  // 0x7018b8
    const X87Reg ratio = X87Reg(near_) / far_;
    std::memset(m, 0, 16 * sizeof(float));
    const X87Reg half = angle * 0.5;  // 0x7018c0 (a double)
    const X87Reg depth = X87_SubR(ratio, 1.0f);
    const double s = X87_Sin(half.v), c = X87_Cos(half.v);
    const X87Reg q = DivRound(s, depth.v);
    const X87Reg shift = q * near_;
    m[0] = (float)c;
    m[5] = (float)c;
    m[10] = (float)q;
    m[14] = (float)(-shift.v);
    if (global<u32>(0x008bcbc8) & 0x200)
        m[15] = 1.0f;
    else
        m[11] = (float)s;
    std::memcpy(&global<u32>(0x0087b1dc), &near_, 4);
    std::memcpy(&global<u32>(0x00768acc), &far_, 4);
    std::memcpy(&global<u32>(0x0087af58), &fov, 4);
}

void __fastcall Mat4_SetFar(float *m, void *, u32 far_) {
    // The kept values go on as they are (as dwords, a signalling NaN too).
    using PerspectiveBits = void(__fastcall *)(float *, void *, u32, u32, u32);
    reinterpret_cast<PerspectiveBits>(&Mat4_Perspective)(m, nullptr, global<u32>(0x0087b1dc), far_, global<u32>(0x0087af58));
    float far_value;
    std::memcpy(&far_value, &far_, 4);
    const float near_ = global<float>(0x0087b1dc);
    X87Reg d = ((X87Reg(far_value) - m[14]) / m[10]) * global<float>(0x008bcc0c);
    if (!(d.v >= near_)) d = near_;
    const X87Reg b = X87Reg(m[11]) * d + m[15];
    const X87Reg a = d * m[10] + m[14];
    View_SetProjection(reinterpret_cast<u8 *>(0x008bcbc8), nullptr, far_, (float)a, (float)b);
}

void __fastcall Mat4_Transform(const float *m, void *, float *out, const float *in, u32 mode) {
    if (mode > 4) return;
    // The vector is read first (the output may overlap it); each component reads its matrix entries after the one before is stored
    // (the output may overlap the matrix). The sums go in the original's order, the destination on the left.
    const X87Reg x = in[0], y = in[1], z = in[2];
    if (mode == 4) {  // the 3x3 transposed
        out[0] = (float)((X87Reg(m[0]) * x + X87Reg(m[2]) * z) + X87Reg(m[1]) * y);
        out[1] = (float)((X87Reg(m[6]) * z + X87Reg(m[5]) * y) + X87Reg(m[4]) * x);
        out[2] = (float)((X87Reg(m[9]) * y + X87Reg(m[8]) * x) + X87Reg(m[10]) * z);
        return;
    }
    if (mode == 1) {  // with the vector's w against the last row
        const X87Reg w = in[3];
        out[0] = (float)(((X87Reg(m[0]) * x + X87Reg(m[8]) * z) + X87Reg(m[4]) * y) + X87Reg(m[12]) * w);
        out[1] = (float)(((X87Reg(m[9]) * z + X87Reg(m[5]) * y) + X87Reg(m[13]) * w) + X87Reg(m[1]) * x);
        for (int c = 2; c < 4; ++c)
            out[c] = (float)(((X87Reg(m[4 + c]) * y + X87Reg(m[8 + c]) * z) + X87Reg(m[c]) * x) + X87Reg(m[12 + c]) * w);
        return;
    }
    // 0: four components plus the last row; 2: three of them; 3: three without the last row
    const int n = mode == 0 ? 4 : 3;
    const bool move = mode != 3;
    X87Reg s = X87Reg(m[0]) * x + X87Reg(m[8]) * z + X87Reg(m[4]) * y;
    out[0] = (float)(move ? s + m[12] : s);
    s = X87Reg(m[9]) * z + X87Reg(m[5]) * y + X87Reg(m[1]) * x;
    out[1] = (float)(move ? s + m[13] : s);
    for (int c = 2; c < n; ++c) {
        s = X87Reg(m[4 + c]) * y + X87Reg(m[8 + c]) * z + X87Reg(m[c]) * x;
        out[c] = (float)(move ? s + m[12 + c] : s);
    }
}
}
