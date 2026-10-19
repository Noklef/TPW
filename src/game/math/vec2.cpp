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

static u32 circles_meet(const s32 *p, const s32 *q) {
    const s32 rp = p[3] > p[4] ? p[3] : p[4];
    const s32 rq = q[3] > q[4] ? q[3] : q[4];
    const s32 length = IntVec2_Length((s32)((u32)p[1] - (u32)q[1]), (s32)((u32)p[2] - (u32)q[2]));
    return length < (s32)((u32)rq + (u32)rp) ? 1 : 0;
}

u32 __cdecl IntArea_Overlaps(const s32 *a, const s32 *b) {
    if (a == b)
        return 1;
    const u32 fa = (u32)a[0] & 0x1e;
    if (fa == 0x1e && ((u32)b[0] & 0x1e) == 0x1e)
        return circles_meet(a, b);
    if ((((u32)b[0] | (u32)a[0]) & 0x1e) != 0x1e)
        return a[2] == b[2] && a[1] == b[1] ? 1 : 0;
    const s32 *p = a, *q = b;
    bool both = false;
    if (fa == 0x1e) {
        p = b;
        q = a;
    } else {
        both = ((u32)b[0] & 0x1e) != 0x1e;
    }
    const u32 flags = (u32)p[0];
    if (!((flags & 2) && q[1] < p[1]) && !((flags & 4) && q[1] > p[1]) && !((flags & 8) && q[2] > p[2]) && !((flags & 0x10) && q[2] < p[2]))
        return 0;
    if (both) {
        const s32 dx = (s32)((u32)p[1] - (u32)q[1]), dy = (s32)((u32)p[2] - (u32)q[2]);
        const u32 ax = ((u32)dx ^ (u32)(dx >> 31)) - (u32)(dx >> 31), ay = ((u32)dy ^ (u32)(dy >> 31)) - (u32)(dy >> 31);
        return ax + ay == 0x1800 ? 1 : 0;
    }
    return circles_meet(p, q);
}
}
