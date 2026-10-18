// Small vector helpers: reimplementation of 0x431480-0x431660.
#include "vector.h"

#include <cmath>

#include "matrix.h"
#include "x87.h"

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
    v[1] = (float)(X87Reg(y) * c - X87Reg(s) * v[2]);
    v[2] = (float)(X87Reg(c) * v[2] + X87Reg(s) * y);
}

void Vec3_RotateYWith(const SinTable &sines, float *v, float angle) {
    float c, s;
    cos_sin(sines, angle, c, s);
    const float x = v[0];
    v[0] = (float)(X87Reg(s) * v[2] + X87Reg(x) * c);
    v[2] = (float)(X87Reg(c) * v[2] - X87Reg(s) * x);
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
    v[0] = (float)(X87Reg(s) * v[2] + X87Reg(x) * c);
    v[2] = (float)(X87Reg(c) * v[2] - X87Reg(x) * s);
}

extern "C" void __cdecl Vec3_RotateX(float *v, float angle) { Vec3_RotateXWith(SinTable_Game(), v, angle); }
extern "C" void __cdecl Vec3_RotateY(float *v, float angle) { Vec3_RotateYWith(SinTable_Game(), v, angle); }
extern "C" void __cdecl Matrix_RotateYAngle(float *m, float angle) {
    Matrix_RotateYAngleWith(SinTable_Game(), m, angle);
}

extern "C" void __fastcall Vec3_Clamp(float *v, void *, float lo, float hi) {
    for (int i = 0; i < 3; i++) {
        const float c = v[i];
        v[i] = X87_Through(c > hi ? hi : !(c >= lo) ? lo : c);  // fcomp, test ah 0x41 / 0x1
    }
}
