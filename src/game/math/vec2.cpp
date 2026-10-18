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

s32 __cdecl IntVec2_Length(s32 x, s32 y) {
    s32 ax = (s32)(((u32)x ^ (u32)(x >> 31)) - (u32)(x >> 31));  // INT_MIN stays INT_MIN
    s32 ay = (s32)(((u32)y ^ (u32)(y >> 31)) - (u32)(y >> 31));
    u32 n = 0;
    while (ax > 0x8000 || ay > 0x7fff) {
        n++;
        ax >>= 1;
        ay >>= 1;
    }
    const s32 sum = (s32)((u32)ax * (u32)ax + (u32)ay * (u32)ay);
    const s32 root = (s32)(long long)X87_Sqrt(X87Reg((double)sum)).v;  // __ftol
    return (s32)((u32)root << (n & 31));
}
}
