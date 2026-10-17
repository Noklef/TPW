#include "intpair.h"

#include "x87.h"

extern "C" {

u32 *__fastcall IntPair_Copy(u32 *pair, void *, const u32 *from) {
    pair[0] = from[0];
    pair[1] = from[1];
    return pair;
}

u32 *__fastcall IntPair_Scaled(const u32 *pair, void *, u32 *out, u32 scale) {
    const u32 first = pair[0] * scale, second = pair[1] * scale;
    out[0] = first;
    out[1] = second;
    return out;
}

s32 *__fastcall IntPair_DivFixed(const s32 *pair, void *, s32 *out, s32 divisor) {
    const s32 y = (s32)(((long long)pair[1] << 16) / divisor);
    const s32 x = (s32)(((long long)pair[0] << 16) / divisor);
    out[0] = x;
    out[1] = y;
    return out;
}

u32 *__fastcall IntPair_Add(const u32 *pair, void *, u32 *out, const u32 *other) {
    const u32 y = other[1] + pair[1], x = other[0] + pair[0];
    out[0] = x;
    out[1] = y;
    return out;
}

u32 *__fastcall IntPair_Sub(const u32 *pair, void *, u32 *out, const u32 *other) {
    const u32 y = pair[1] - other[1], x = pair[0] - other[0];
    out[0] = x;
    out[1] = y;
    return out;
}

s32 *__fastcall IntPair_MulFixed(const s32 *pair, void *, s32 *out, s32 scale) {
    const s32 y = (s32)(((long long)pair[1] * scale) >> 16);
    const s32 x = (s32)(((long long)pair[0] * scale) >> 16);
    out[1] = y;
    out[0] = x;
    return out;
}

u32 __cdecl Math_ISqrt(u32 n) {
    return (u32)(long long)X87_Sqrt(X87Reg((double)n)).v;  // fild qword (n, 0); fsqrt; __ftol
}

u32 *__fastcall IntPair_Neg(const u32 *pair, void *, u32 *out) {
    const u32 y = 0u - pair[1], x = 0u - pair[0];
    out[0] = x;
    out[1] = y;
    return out;
}

s32 __fastcall IntPair_ApproxLength(const s32 *pair) {
    const u32 ax = pair[0] < 0 ? 0u - (u32)pair[0] : (u32)pair[0];
    const u32 ay = pair[1] < 0 ? 0u - (u32)pair[1] : (u32)pair[1];
    const s32 shorter = (s32)ax <= (s32)ay ? (s32)ax : (s32)ay;
    return (s32)(ay - (u32)(shorter >> 1) + ax);
}

u32 *__cdecl IntPair_Perp(u32 *out, const u32 *pair) {
    const u32 x = pair[0], y = pair[1];
    out[0] = 0u - y;
    out[1] = x;
    return out;
}

u32 *__cdecl IntPair_FromFixed(u32 *out, u32 x, u32 y) {
    out[0] = x >> 16;
    out[1] = y >> 16;
    return out;
}

u32 *__cdecl IntPair_ToFixed(u32 *out, u32 x, u32 y) {
    out[0] = x << 16;
    out[1] = y << 16;
    return out;
}

u32 *__fastcall IntPair_Ctor(u32 *pair) {
    pair[0] = 0;
    pair[1] = 0;
    return pair;
}
}
