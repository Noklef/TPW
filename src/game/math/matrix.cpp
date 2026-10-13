// The matrix library: reimplementation of 0x46f150-0x46fd40.
#include "matrix.h"

#include <cmath>
#include <cstring>

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
        return std::atan2((double)m[8], (double)m[10]);
    return std::atan2(-(double)m[2], (double)m[0]);
}

extern "C" double __cdecl Matrix_Roll(const float *m) { return std::atan2(-(double)m[4], (double)m[5]); }
extern "C" double __cdecl Vec3_AngleX(const float *v) { return std::atan2(-(double)v[1], (double)v[2]); }
extern "C" double __cdecl Vec3_AngleY(const float *v) { return std::atan2((double)v[0], (double)v[2]); }
extern "C" double __cdecl Vec3_AngleZ(const float *v) { return std::atan2(-(double)v[0], (double)v[1]); }

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
