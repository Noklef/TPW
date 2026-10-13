// Small vector helpers: reimplementation of 0x431480-0x431660.
#include "vector.h"

#include <cmath>

#include "matrix.h"

namespace {

void cos_sin(const SinTable &sines, float angle, float &c, float &s) {
    s = SinTable_At(sines, angle * sines.scale);
    c = SinTable_At(sines, (angle - -1.57079637f) * sines.scale);  // 0x6fdeb0
}

}  // namespace

void Vec3_RotateXWith(const SinTable &sines, float *v, float angle) {
    float c, s;
    cos_sin(sines, angle, c, s);
    const float y = v[1];
    v[1] = y * c - s * v[2];
    v[2] = c * v[2] + s * y;
}

void Vec3_RotateYWith(const SinTable &sines, float *v, float angle) {
    float c, s;
    cos_sin(sines, angle, c, s);
    const float x = v[0];
    v[0] = x * c + s * v[2];
    v[2] = c * v[2] - s * x;
}

void Matrix_RotateYAngleWith(const SinTable &sines, float *m, float angle) {
    float c, s;
    cos_sin(sines, angle, c, s);
    Matrix_RotateY(m, c, s);
}

extern "C" double __cdecl Float_Modf(float x, float *whole) {
    double w;
    const double frac = std::modf((double)x, &w);
    if (whole)
        *whole = (float)w;
    return frac;
}

extern "C" void __cdecl Vec3_RotateYCS(float *v, float c, float s) {
    const float x = v[0];
    v[0] = x * c + s * v[2];
    v[2] = c * v[2] - x * s;
}

extern "C" void __cdecl Vec3_RotateX(float *v, float angle) { Vec3_RotateXWith(SinTable_Game(), v, angle); }
extern "C" void __cdecl Vec3_RotateY(float *v, float angle) { Vec3_RotateYWith(SinTable_Game(), v, angle); }
extern "C" void __cdecl Matrix_RotateYAngle(float *m, float angle) {
    Matrix_RotateYAngleWith(SinTable_Game(), m, angle);
}
