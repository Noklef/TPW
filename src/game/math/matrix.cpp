// The matrix library: reimplementation of 0x46f150-0x470940.
#include "matrix.h"

#include <cmath>
#include <cstdint>
#include <cstring>

#include "x87.h"

namespace {

// gEulerOrders (0x74da38)
const char *const kEulerOrders[16] = {"FUK", "X",  "Y",  "Z",   "XY",  "YX",  "YZ",  "ZY",
                                      "XZ",  "ZX", "XYZ", "XZY", "YXZ", "YZX", "ZXY", "ZYX"};

// One rotation of rows [0, rows) on the pair of columns (i, j): i' = i c - j s, j' = i s + j c.
// In the x87's registers; about y the original sums j' the other way round (j c + i s).
void rotate(float *m, int rows, int i, int j, float c, float s) {
    for (int r = 0; r < rows; ++r) {
        float *row = m + 4 * r;
        const X87Reg a = row[i], b = row[j];
        row[i] = (float)(a * c - b * s);
        row[j] = (float)(j == 0 ? b * c + a * s : a * s + b * c);
    }
}

template <typename CosSin>
void rotate_euler(float *m, s32 order, const float *angles, CosSin cos_sin) {
    for (const char *p = kEulerOrders[order]; *p; ++p) {
        if (*p != 'X' && *p != 'Y' && *p != 'Z')
            continue;
        float c, s;
        cos_sin(*angles++, c, s);
        if (*p == 'X')
            Matrix_RotateX(m, c, s);
        else if (*p == 'Y')
            Matrix_RotateY(m, c, s);
        else
            Matrix_RotateZ(m, c, s);
    }
}

// fsqrt: a NaN comes through.
X87Reg sqrt87(X87Reg x) { return x.v != x.v ? x : X87Reg(X87_Round(std::sqrt(x.v))); }

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

// Bits 16 to 47 of the 64-bit product.
s32 Shifted(s32 a, s32 b) { return (s32)(u32)((uint64_t)((int64_t)a * (int64_t)b) >> 16); }

}  // namespace

extern "C" void __cdecl Matrix_Identity(float *m) {
    std::memset(m, 0, 16 * sizeof(float));
    m[0] = m[5] = m[10] = m[15] = 1.0f;
}

extern "C" void __cdecl Matrix_ResetRotation(float *m) {
    m[15] = m[10] = m[5] = m[0] = 1.0f;
    m[11] = m[9] = m[8] = m[7] = m[6] = m[4] = m[3] = m[2] = m[1] = 0.0f;
}

extern "C" double __cdecl Matrix_Heading(const float *m) {
    if (m[9] <= 0.9f && !(m[9] < -0.9f))
        return X87_Atan2((double)m[8], (double)m[10]);
    return X87_Atan2(-(double)m[2], (double)m[0]);
}

extern "C" double __cdecl Matrix_Roll(const float *m) { return X87_Atan2(-(double)m[4], (double)m[5]); }
extern "C" double __cdecl Vec3_AngleX(const float *v) { return X87_Atan2(-(double)v[1], (double)v[2]); }
extern "C" double __cdecl Vec3_AngleY(const float *v) { return X87_Atan2((double)v[0], (double)v[2]); }
extern "C" double __cdecl Vec3_AngleZ(const float *v) { return X87_Atan2(-(double)v[0], (double)v[1]); }

// About x: (y, z) turn; about y: (z, x); about z: (x, y).
extern "C" void __cdecl Matrix_RotateX4(float *m, float c, float s) { rotate(m, 4, 1, 2, c, s); }
extern "C" void __cdecl Matrix_RotateY4(float *m, float c, float s) { rotate(m, 4, 2, 0, c, s); }
extern "C" void __cdecl Matrix_RotateZ4(float *m, float c, float s) { rotate(m, 4, 0, 1, c, s); }
extern "C" void __cdecl Matrix_RotateX(float *m, float c, float s) { rotate(m, 3, 1, 2, c, s); }
extern "C" void __cdecl Matrix_RotateY(float *m, float c, float s) { rotate(m, 3, 2, 0, c, s); }
extern "C" void __cdecl Matrix_RotateZ(float *m, float c, float s) { rotate(m, 3, 0, 1, c, s); }

void Matrix_RotateEulerWith(const SinTable &sines, float *m, s32 order, const float *angles) {
    rotate_euler(m, order, angles, [&](float a, float &c, float &s) {
        c = SinTable_At(sines, (a - -1.57079637f) * sines.scale);  // 0x6feb80
        s = SinTable_At(sines, a * sines.scale);
    });
}

extern "C" void __cdecl Matrix_RotateEulerFast(float *m, s32 order, const float *angles) {
    Matrix_RotateEulerWith(SinTable_Game(), m, order, angles);
}

extern "C" void __cdecl Matrix_RotateEuler(float *m, s32 order, const float *angles) {
    rotate_euler(m, order, angles, [](float a, float &c, float &s) {
        c = (float)X87_Cos((double)a);
        s = (float)X87_Sin((double)a);
    });
}

// The three lengths first; then each row's factor, read after the rows before it are
// stored. Sic: x's factor goes through a float store, y's length does, and y's factor
// reaches m[4] from the register but m[5] and m[6] through a float store.
extern "C" void __cdecl Matrix_SetAxisLengths(float *m, const float *x, const float *y, const float *z) {
    const auto length = [](const float *row) {
        const X87Reg a = row[0], b = row[1], c = row[2];
        return sqrt87(a * a + b * b + c * c);
    };
    const X87Reg len0 = length(m), len2 = length(m + 8);
    const float len1 = (float)length(m + 4);
    const float f0 = (float)(X87Reg(*x) / len0);
    m[0] = (float)(X87Reg(m[0]) * f0);
    m[1] = (float)(X87Reg(f0) * m[1]);
    m[2] = (float)(X87Reg(f0) * m[2]);
    const X87Reg f1 = X87Reg(*y) / len1;
    const float f1f = (float)f1;
    m[4] = (float)(f1 * m[4]);
    m[5] = (float)(X87Reg(f1f) * m[5]);
    m[6] = (float)(X87Reg(f1f) * m[6]);
    const X87Reg f2 = X87Reg(*z) / len2;
    for (int c = 8; c < 11; ++c)
        m[c] = (float)(f2 * m[c]);
}

extern "C" void __cdecl Matrix_ScaleAxesV(float *m, const float *s) {
    for (int c = 0; c < 3; ++c)
        m[c] = (float)(X87Reg(s[0]) * m[c]);
    m[4] = (float)(X87Reg(s[1]) * m[4]);
    m[5] = (float)(X87Reg(m[5]) * s[1]);
    m[6] = (float)(X87Reg(m[6]) * s[1]);
    m[8] = (float)(X87Reg(s[2]) * m[8]);
    m[9] = (float)(X87Reg(m[9]) * s[2]);
    m[10] = (float)(X87Reg(m[10]) * s[2]);
}

extern "C" void __cdecl Matrix_ScaleAxes(float *m, float x, float y, float z) {
    m[0] = (float)(X87Reg(m[0]) * x);
    m[1] = (float)(X87Reg(x) * m[1]);
    m[2] = (float)(X87Reg(x) * m[2]);
    for (int c = 4; c < 7; ++c)
        m[c] = (float)(X87Reg(y) * m[c]);
    for (int c = 8; c < 11; ++c)
        m[c] = (float)(X87Reg(z) * m[c]);
}

extern "C" void __cdecl Matrix_FromPitchYaw(float *m, float pitch, float yaw) {
    const float s = (float)X87_Sin((double)pitch), c = (float)X87_Cos((double)pitch);
    m[8] = m[4] = m[2] = m[1] = 0.0f;
    m[0] = 1.0f;
    m[5] = c;
    m[6] = s;
    m[9] = -s;
    m[10] = c;
    Matrix_RotateY(m, (float)X87_Cos((double)yaw), (float)X87_Sin((double)yaw));
}

extern "C" void __cdecl Matrix_SetAxes(float *m, const float *x, const float *y, const float *z) {
    if (x) std::memcpy(m, x, 3 * sizeof(float));
    if (y) std::memcpy(m + 4, y, 3 * sizeof(float));
    if (z) std::memcpy(m + 8, z, 3 * sizeof(float));
}

extern "C" void __cdecl Matrix_TransformPoint(const float *m, float *v) {
    const X87Reg x = v[0], y = v[1], z = v[2];
    v[0] = (float)(x * m[0] + X87Reg(m[8]) * z + X87Reg(m[4]) * y + m[12]);
    v[1] = (float)(X87Reg(m[9]) * z + X87Reg(m[5]) * y + X87Reg(m[1]) * x + m[13]);
    v[2] = (float)(X87Reg(m[10]) * z + X87Reg(m[6]) * y + X87Reg(m[2]) * x + m[14]);
}

extern "C" void __cdecl Matrix_TransformVector(const float *m, float *v) {
    const X87Reg x = v[0], y = v[1], z = v[2];
    v[0] = (float)(x * m[0] + X87Reg(m[8]) * z + X87Reg(m[4]) * y);
    v[1] = (float)(X87Reg(m[9]) * z + X87Reg(m[5]) * y + X87Reg(m[1]) * x);
    v[2] = (float)(X87Reg(m[10]) * z + X87Reg(m[6]) * y + X87Reg(m[2]) * x);
}

extern "C" void __cdecl Matrix_Mul(float *a, const float *b) {
    float l[16];
    std::memcpy(l, a, sizeof l);
    // Rows in order, each element stored as soon as it is made; b is read live, so with
    // a == b the later rows see the earlier ones' results, as in the original.
    for (int r = 0; r < 4; ++r) {
        const float *lr = l + 4 * r;
        float *ar = a + 4 * r;
        for (int c = 0; c < 3; ++c) {
            X87Reg v = c == 0 ? X87Reg(lr[1]) * b[4] + X87Reg(lr[2]) * b[8] + X87Reg(lr[0]) * b[0]
                              : X87Reg(lr[0]) * b[c] + X87Reg(lr[2]) * b[8 + c] + X87Reg(lr[1]) * b[4 + c];
            if (r == 3)
                v = v + b[12 + c];
            ar[c] = (float)v;
        }
    }
}

extern "C" void __cdecl Matrix_PreMul(const float *a, float *b) {
    float l[16];
    std::memcpy(l, b, sizeof l);
    for (int c = 0; c < 3; ++c)  // stored by columns
        for (int r = 0; r < 4; ++r) {
            const float *ar = a + 4 * r;
            X87Reg v = X87Reg(l[8 + c]) * ar[2] + X87Reg(l[4 + c]) * ar[1] + X87Reg(l[c]) * ar[0];
            if (r == 3)
                v = v + l[12 + c];
            b[4 * r + c] = (float)v;
        }
}

extern "C" void __cdecl Matrix_PreMul33(const float *r, float *m) {
    for (int c = 0; c < 3; ++c) {  // by columns; every read is of the old column
        const float a = m[c], b = m[4 + c], d = m[8 + c];
        m[c] = (float)((X87Reg(d) * r[2] + X87Reg(b) * r[1]) + X87Reg(a) * r[0]);
        m[4 + c] = (float)((X87Reg(b) * r[4] + X87Reg(d) * r[5]) + X87Reg(a) * r[3]);
        m[8 + c] = (float)((X87Reg(d) * r[8] + X87Reg(a) * r[6]) + X87Reg(b) * r[7]);
    }
}

extern "C" void __cdecl Matrix_PreMul3(const float *a, float *b) {
    float l[16];
    std::memcpy(l, b, sizeof l);
    for (int c = 0; c < 3; ++c) {  // stored by columns
        b[c] = (float)(X87Reg(l[4 + c]) * a[1] + X87Reg(l[8 + c]) * a[2] + X87Reg(l[c]) * a[0]);
        for (int r = 1; r < 3; ++r) {
            const float *ar = a + 4 * r;
            b[4 * r + c] = (float)(X87Reg(l[c]) * ar[0] + X87Reg(l[4 + c]) * ar[1] + X87Reg(l[8 + c]) * ar[2]);
        }
    }
}

// The flag tests below copy fcom's: an unordered (NaN) compare sets C0, C2 and C3 together.

extern "C" void __cdecl Matrix_LUDecompose(float *a, s32 *index, float *parity) {
    float vv[4];
    *parity = 1.0f;
    for (int i = 0; i < 4; ++i) {
        float big = 0.0f;
        for (int j = 0; j < 4; ++j) {
            const float t = std::fabs(a[4 * i + j]);
            if (t > big)
                big = t;
        }
        vv[i] = (float)(X87Reg(1.0f) / big);  // no singular-matrix check: 1 / 0 is inf
    }
    s32 imax = 0;  // the original's is uninitialised; only an all-NaN column leaves it unset
    // The sums and dum stay in the registers; big is a float.
    for (int j = 0; j < 4; ++j) {
        for (int i = 0; i < j; ++i) {
            X87Reg sum = a[4 * i + j];
            for (int k = 0; k < i; ++k)
                sum = sum - X87Reg(a[4 * k + j]) * a[4 * i + k];
            a[4 * i + j] = (float)sum;
        }
        float big = 0.0f;
        for (int i = j; i < 4; ++i) {
            X87Reg sum = a[4 * i + j];
            for (int k = 0; k < j; ++k)
                sum = sum - X87Reg(a[4 * k + j]) * a[4 * i + k];
            const X87Reg dum = X87Reg(std::fabs(sum.v)) * vv[i];
            a[4 * i + j] = (float)sum;
            if (dum.v >= big) {
                big = (float)dum;
                imax = i;
            }
        }
        if (j != imax) {
            for (int k = 0; k < 4; ++k) {  // the row j value goes through fld/fstp
                const float t = a[4 * imax + k];
                a[4 * imax + k] = a[4 * j + k];
                a[4 * j + k] = X87_Through(t);
            }
            *parity = -*parity;
            vv[imax] = vv[j];
        }
        index[j] = imax;
        if (!(a[5 * j] < 0.0f || a[5 * j] > 0.0f))  // zero or NaN
            a[5 * j] = 1e-20f;                      // 0x1e3ce508
        if (j != 3) {
            const float dum = (float)(X87Reg(1.0f) / a[5 * j]);
            for (int i = j + 1; i < 4; ++i)
                a[4 * i + j] = (float)(X87Reg(dum) * a[4 * i + j]);
        }
    }
}

extern "C" void __cdecl Matrix_LUBackSubstitute(const float *a, const s32 *index, float *b) {
    s32 ii = -1;
    for (int i = 0; i < 4; ++i) {
        const s32 ip = index[i];
        X87Reg sum = b[ip];
        b[ip] = b[i];
        if (ii >= 0) {
            for (int j = ii; j < i; ++j)
                sum = sum - X87Reg(b[j]) * a[4 * i + j];
        } else if (sum.v < 0.0 || sum.v > 0.0) {
            ii = i;
        }
        b[i] = (float)sum;
    }
    for (int i = 3; i >= 0; --i) {
        X87Reg sum = b[i];
        for (int j = i + 1; j < 4; ++j)
            sum = sum - X87Reg(a[4 * i + j]) * b[j];
        b[i] = (float)(sum / a[5 * i]);
    }
}

extern "C" float *__cdecl Matrix_Invert(float *dst, const float *src) {
    float lu[16], inv[16], parity;
    s32 index[4];
    std::memcpy(lu, src, sizeof lu);
    Matrix_LUDecompose(lu, index, &parity);
    for (int j = 0; j < 4; ++j) {
        float col[4] = {0.0f, 0.0f, 0.0f, 0.0f};
        col[j] = 1.0f;
        Matrix_LUBackSubstitute(lu, index, col);
        for (int i = 0; i < 4; ++i)
            inv[4 * i + j] = col[i];
    }
    std::memcpy(dst, inv, sizeof inv);
    return dst;
}

extern "C" void __cdecl Sort_MergeByKey(s32 n, s32 *a, s32 *b, s32 depth, const u8 *keys) {
    // Odd depths merge from b into a, even ones from a into b.
    const s32 *src = depth & 1 ? b : a;
    s32 *out = depth & 1 ? a : b;
    const s32 half = n / 2;
    if (n <= 1) {
        if (n != 0)
            *out = *a;
        return;
    }
    Sort_MergeByKey(half, a, b, depth + 1, keys);
    Sort_MergeByKey(n - half, a + half, b + half, depth + 1, keys);
    const s32 *left = src, *right = src + half;
    s32 nleft = half, nright = n - half;
    for (s32 k = n; k != 0; --k) {
        auto key = [&](s32 offset) { return *reinterpret_cast<const float *>(keys + offset); };
        // The left one unless the right key is smaller (or either is NaN): stable, ascending.
        if (nleft != 0 && (nright == 0 || key(*right) >= key(*left))) {
            *out++ = *left++;
            --nleft;
        } else {
            *out++ = *right++;
            --nright;
        }
    }
}

extern "C" void __cdecl Matrix_LookAlong(float *m, const float *dir) {
    // Sic: each of c0..c2 squares as register times float store but c0 (register squared),
    // and x1, x2 go through float stores for one of their two uses.
    const X87Reg y0 = m[4], y1 = m[5], y2 = m[6];
    const X87Reg c0 = y1 * dir[2] - y2 * dir[1];
    const X87Reg c1 = y2 * dir[0] - y0 * dir[2];
    const X87Reg c2 = y0 * dir[1] - y1 * dir[0];
    const float c0f = (float)c0, c1f = (float)c1, c2f = (float)c2;
    const float len = (float)sqrt87(c0 * c0 + c1 * c1f + c2 * c2f);
    const X87Reg x0 = X87Reg(c0f) / len, x1 = X87Reg(c1f) / len, x2 = X87Reg(c2f) / len;
    const float x1f = (float)x1, x2f = (float)x2;
    const float n0 = (float)(x2 * dir[1] - x1 * dir[2]);
    const float n1 = (float)(x0 * dir[2] - X87Reg(x2f) * dir[0]);
    const float n2 = (float)(X87Reg(x1f) * dir[0] - x0 * dir[1]);
    m[0] = (float)x0;
    m[1] = x1f;
    m[2] = x2f;
    m[4] = n0;
    m[5] = n1;
    m[6] = n2;
    if (dir) {
        m[8] = dir[0];
        m[9] = dir[1];
        m[10] = dir[2];
    }
}

extern "C" double __cdecl Vec3_Distance(const float *a, const float *b) {
    const X87Reg dx = X87Reg(a[0]) - b[0], dy = X87Reg(a[1]) - b[1];
    const float dz = (float)(X87Reg(a[2]) - b[2]);
    const X87Reg sum = (dx * dx + dy * (float)dy) + X87Reg(dz) * dz;
    return sum.v != sum.v ? sum.v : X87_Round(std::sqrt(sum.v));  // fsqrt keeps a NaN
}

extern "C" {

void __fastcall Matrix2_Transform(u8 *matrix, void *, s32 *out, const s32 *in) {
    const s32 first = Shifted(in[0], at<s32>(matrix, 0x24));
    const s32 second = Shifted(in[1], at<s32>(matrix, 0x2c));
    const s32 third = Shifted(in[0], at<s32>(matrix, 0x20));
    const s32 fourth = Shifted(in[1], at<s32>(matrix, 0x28));
    out[1] = second + first;
    out[0] = fourth + third;
}

}  // extern "C"
