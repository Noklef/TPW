// The matrix library: reimplementation of 0x46f150-0x470940.
#include "matrix.h"

#include <cmath>
#include <cstring>

#include "x87.h"

namespace {

// gEulerOrders (0x74da38)
const char *const kEulerOrders[16] = {"FUK", "X",  "Y",  "Z",   "XY",  "YX",  "YZ",  "ZY",
                                      "XZ",  "ZX", "XYZ", "XZY", "YXZ", "YZX", "ZXY", "ZYX"};

// One rotation of rows [0, rows) on the pair of columns (i, j): i' = i c - j s, j' = i s + j c.
void rotate(float *m, int rows, int i, int j, float c, float s) {
    for (int r = 0; r < rows; ++r) {
        float *row = m + 4 * r;
        const float a = row[i], b = row[j];
        row[i] = a * c - b * s;
        row[j] = a * s + b * c;
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

void scale_row(float *row, float f) {
    row[0] = row[0] * f;
    row[1] = f * row[1];
    row[2] = f * row[2];
}

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
        c = (float)std::cos((double)a);
        s = (float)std::sin((double)a);
    });
}

extern "C" void __cdecl Matrix_SetAxisLengths(float *m, const float *x, const float *y, const float *z) {
    const float *len[3] = {x, y, z};
    for (int r = 0; r < 3; ++r) {
        float *row = m + 4 * r;
        scale_row(row, *len[r] / std::sqrt((row[0] * row[0] + row[1] * row[1]) + row[2] * row[2]));
    }
}

extern "C" void __cdecl Matrix_ScaleAxesV(float *m, const float *s) {
    for (int r = 0; r < 3; ++r)
        scale_row(m + 4 * r, s[r]);
}

extern "C" void __cdecl Matrix_ScaleAxes(float *m, float x, float y, float z) {
    scale_row(m, x);
    scale_row(m + 4, y);
    scale_row(m + 8, z);
}

extern "C" void __cdecl Matrix_FromPitchYaw(float *m, float pitch, float yaw) {
    const float s = (float)std::sin((double)pitch), c = (float)std::cos((double)pitch);
    m[8] = m[4] = m[2] = m[1] = 0.0f;
    m[0] = 1.0f;
    m[5] = c;
    m[6] = s;
    m[9] = -s;
    m[10] = c;
    Matrix_RotateY(m, (float)std::cos((double)yaw), (float)std::sin((double)yaw));
}

extern "C" void __cdecl Matrix_SetAxes(float *m, const float *x, const float *y, const float *z) {
    if (x) std::memcpy(m, x, 3 * sizeof(float));
    if (y) std::memcpy(m + 4, y, 3 * sizeof(float));
    if (z) std::memcpy(m + 8, z, 3 * sizeof(float));
}

extern "C" void __cdecl Matrix_TransformPoint(const float *m, float *v) {
    const float x = v[0], y = v[1], z = v[2];
    v[0] = ((x * m[0] + z * m[8]) + y * m[4]) + m[12];
    v[1] = ((z * m[9] + y * m[5]) + x * m[1]) + m[13];
    v[2] = ((z * m[10] + y * m[6]) + x * m[2]) + m[14];
}

extern "C" void __cdecl Matrix_TransformVector(const float *m, float *v) {
    const float x = v[0], y = v[1], z = v[2];
    v[0] = (x * m[0] + z * m[8]) + y * m[4];
    v[1] = (z * m[9] + y * m[5]) + x * m[1];
    v[2] = (z * m[10] + y * m[6]) + x * m[2];
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
            float v = c == 0 ? (lr[1] * b[4] + lr[2] * b[8]) + lr[0] * b[0]
                             : (lr[0] * b[c] + lr[2] * b[8 + c]) + lr[1] * b[4 + c];
            if (r == 3)
                v = v + b[12 + c];
            ar[c] = v;
        }
    }
}

extern "C" void __cdecl Matrix_PreMul(const float *a, float *b) {
    float l[16];
    std::memcpy(l, b, sizeof l);
    for (int c = 0; c < 3; ++c)  // stored by columns
        for (int r = 0; r < 4; ++r) {
            const float *ar = a + 4 * r;
            float v = (l[8 + c] * ar[2] + l[4 + c] * ar[1]) + l[c] * ar[0];
            if (r == 3)
                v = v + l[12 + c];
            b[4 * r + c] = v;
        }
}

extern "C" void __cdecl Matrix_PreMul33(const float *r, float *m) {
    for (int c = 0; c < 3; ++c) {  // by columns; every read is of the old column
        const float a = m[c], b = m[4 + c], d = m[8 + c];
        m[c] = (d * r[2] + b * r[1]) + a * r[0];
        m[4 + c] = (b * r[4] + d * r[5]) + a * r[3];
        m[8 + c] = (d * r[8] + a * r[6]) + b * r[7];
    }
}

extern "C" void __cdecl Matrix_PreMul3(const float *a, float *b) {
    float l[16];
    std::memcpy(l, b, sizeof l);
    for (int c = 0; c < 3; ++c) {  // stored by columns
        b[c] = (l[4 + c] * a[1] + l[8 + c] * a[2]) + l[c] * a[0];
        for (int r = 1; r < 3; ++r) {
            const float *ar = a + 4 * r;
            b[4 * r + c] = (l[c] * ar[0] + l[4 + c] * ar[1]) + l[8 + c] * ar[2];
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
        vv[i] = 1.0f / big;  // no singular-matrix check: 1 / 0 is inf
    }
    s32 imax = 0;  // the original's is uninitialised; only an all-NaN column leaves it unset
    for (int j = 0; j < 4; ++j) {
        for (int i = 0; i < j; ++i) {
            float sum = a[4 * i + j];
            for (int k = 0; k < i; ++k)
                sum = sum - a[4 * k + j] * a[4 * i + k];
            a[4 * i + j] = sum;
        }
        float big = 0.0f;
        for (int i = j; i < 4; ++i) {
            float sum = a[4 * i + j];
            for (int k = 0; k < j; ++k)
                sum = sum - a[4 * k + j] * a[4 * i + k];
            const float dum = std::fabs(sum) * vv[i];
            a[4 * i + j] = sum;
            if (dum >= big) {
                big = dum;
                imax = i;
            }
        }
        if (j != imax) {
            for (int k = 0; k < 4; ++k) {
                const float t = a[4 * imax + k];
                a[4 * imax + k] = a[4 * j + k];
                a[4 * j + k] = t;
            }
            *parity = -*parity;
            vv[imax] = vv[j];
        }
        index[j] = imax;
        if (!(a[5 * j] < 0.0f || a[5 * j] > 0.0f))  // zero or NaN
            a[5 * j] = 1e-20f;                      // 0x1e3ce508
        if (j != 3) {
            const float dum = 1.0f / a[5 * j];
            for (int i = j + 1; i < 4; ++i)
                a[4 * i + j] = dum * a[4 * i + j];
        }
    }
}

extern "C" void __cdecl Matrix_LUBackSubstitute(const float *a, const s32 *index, float *b) {
    s32 ii = -1;
    for (int i = 0; i < 4; ++i) {
        const s32 ip = index[i];
        float sum = b[ip];
        b[ip] = b[i];
        if (ii >= 0) {
            for (int j = ii; j < i; ++j)
                sum = sum - b[j] * a[4 * i + j];
        } else if (sum < 0.0f || sum > 0.0f) {
            ii = i;
        }
        b[i] = sum;
    }
    for (int i = 3; i >= 0; --i) {
        float sum = b[i];
        for (int j = i + 1; j < 4; ++j)
            sum = sum - a[4 * i + j] * b[j];
        b[i] = sum / a[5 * i];
    }
}

extern "C" void __cdecl Matrix_Invert(float *dst, const float *src) {
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
    const float *y = m + 4;
    const float c0 = y[1] * dir[2] - y[2] * dir[1];
    const float c1 = y[2] * dir[0] - y[0] * dir[2];
    const float c2 = y[0] * dir[1] - y[1] * dir[0];
    const float len = std::sqrt((c0 * c0 + c1 * c1) + c2 * c2);
    const float x0 = c0 / len, x1 = c1 / len, x2 = c2 / len;
    const float y0 = x2 * dir[1] - x1 * dir[2];
    const float y1 = x0 * dir[2] - x2 * dir[0];
    const float y2 = x1 * dir[0] - x0 * dir[1];
    m[0] = x0;
    m[1] = x1;
    m[2] = x2;
    m[4] = y0;
    m[5] = y1;
    m[6] = y2;
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
