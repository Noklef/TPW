#include "vec2.h"

#include "x87.h"

extern "C" {

void __fastcall Vec2_Normalize(float *v) {
    const float x = v[0], y = v[1];
    X87Reg squared = X87Reg(y) * y + X87Reg(x) * x;
    if (squared.v == 0 || squared.v != squared.v) squared = 1.0;  // 0x70189c, 0x701890
    const X87Reg scale = X87Reg(1.0f) / X87_Sqrt(squared);
    const X87Reg nx = X87Reg(v[0]) * scale, ny = scale * v[1];
    v[0] = (float)nx;
    v[1] = (float)ny;
}
}
