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

u32 __cdecl IntArea_Contains(s32 x, s32 y, const s32 *area) {
    if (!area)
        return 0;
    const s32 dx = (s32)((u32)area[1] - (u32)x);
    const s32 dy = (s32)((u32)area[2] - (u32)y);
    const u32 side = (dx >= 0 ? 2u : 4u) | (dy >= 0 ? 0x10u : 8u);
    if (dx == 0 && dy == 0)
        return 1;
    const u32 flags = (u32)area[0];
    if ((side & flags) != side)
        return 0;
    if (flags & 0x20) {
        const s32 ax = (s32)(((u32)dx ^ (u32)(dx >> 31)) - (u32)(dx >> 31));  // (INT_MIN stays INT_MIN)
        const s32 ay = (s32)(((u32)dy ^ (u32)(dy >> 31)) - (u32)(dy >> 31));
        return ax <= area[3] && ay <= area[4] ? 1 : 0;
    }
    return IntVec2_Length(dx, dy) <= area[3] ? 1 : 0;
}
}
