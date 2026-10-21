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

u32 *__fastcall IntPair_Rotated(const u32 *pair, void *, u32 *out, s32 angle) {
    const u32 x = pair[0], y = pair[1];
    switch (angle % 360) {
    case 90:
    case -270:
        out[0] = y;
        out[1] = 0 - x;
        return out;
    case 180:
    case -180:
        out[0] = 0 - x;
        out[1] = 0 - y;
        return out;
    case 270:
    case -90:
        out[0] = 0 - y;
        out[1] = x;
        return out;
    default:   // (0, or another reported to the empty Debug_Printf)
        out[0] = pair[0];
        out[1] = pair[1];
        return out;
    }
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

s32 *__cdecl IntPair_Truncate(s32 *out, const s32 *pair, s32 limit) {
    const u32 ax = pair[0] < 0 ? 0u - (u32)pair[0] : (u32)pair[0], ay = pair[1] < 0 ? 0u - (u32)pair[1] : (u32)pair[1];  // (INT_MIN stays)
    const u32 both = ay | ax;
    s32 measured;
    if (!(both & 0xfffff000))
        measured = (s32)Math_ISqrt(ax * ax + ay * ay);
    else if (!(both & 0xffc00000))
        measured = (s32)(Math_ISqrt((ax >> 8) * (ax >> 8) + (ay >> 8) * (ay >> 8)) << 8);
    else
        measured = (s32)(Math_ISqrt((ax >> 18) * (ax >> 18) + (ay >> 18) * (ay >> 18)) << 18);
    if (measured < limit) {
        out[0] = pair[0];
        out[1] = pair[1];
        return out;
    }
    // limit << 16 (as 64 bits) / measured in one IDIV, as the original's: it faults (sic) on 0 or a quotient past 31 bits.
    const u32 dividendLow = (u32)limit << 16, dividendHigh = (u32)(limit >> 16);
    s32 scale;
    __asm {
        mov eax, dividendLow
        mov edx, dividendHigh
        idiv measured
        mov scale, eax
    }
    const s32 y = (s32)(((long long)pair[1] * scale) >> 16);
    const s32 x = (s32)(((long long)pair[0] * scale) >> 16);
    out[0] = x;
    out[1] = y;
    return out;
}

s32 *__cdecl IntPair_Normalize(s32 *out, const s32 *pair) {
    const s32 ax = pair[0] < 0 ? (s32)(0u - (u32)pair[0]) : pair[0], ay = pair[1] < 0 ? (s32)(0u - (u32)pair[1]) : pair[1];  // (INT_MIN stays)
    const u32 both = (u32)ay | (u32)ax;
    if (!(both & 0xfffff000)) {
        const s32 root = (s32)Math_ISqrt(((u32)ax * (u32)ax + (u32)ay * (u32)ay) << 6);
        const s32 scale = root ? 0x80000 / root : 0;
        const u32 y = (u32)scale * (u32)pair[1], x = (u32)pair[0] * (u32)scale;
        out[0] = (s32)x;
        out[1] = (s32)y;
        return out;
    }
    s32 scale = 0;
    if (!(both & 0xffc00000)) {
        const s32 x8 = ax >> 8, y8 = ay >> 8;
        const s32 root = (s32)Math_ISqrt((u32)x8 * (u32)x8 + (u32)y8 * (u32)y8);
        if (root) scale = 0x1000000 / root;
    } else {
        const s32 x18 = ax >> 18, y18 = ay >> 18;
        const s32 root = (s32)Math_ISqrt((u32)x18 * (u32)x18 + (u32)y18 * (u32)y18);
        if (root) scale = 0x4000 / root;
    }
    const s32 y = (s32)(((long long)pair[1] * scale) >> 16);
    const s32 x = (s32)(((long long)pair[0] * scale) >> 16);
    out[0] = x;
    out[1] = y;
    return out;
}

void __cdecl IntPair_ZeroAtExit() {}

namespace {
void markDestroyed(u8 bit) {
    u8 &flags = *reinterpret_cast<u8 *>(0x007cdbe0);
    if (!(flags & bit)) flags = (u8)(flags | bit);
}
}  // namespace

void __cdecl IntPair_NorthAtExit() { markDestroyed(0x1); }
void __cdecl IntPair_NorthEastAtExit() { markDestroyed(0x2); }
void __cdecl IntPair_EastAtExit() { markDestroyed(0x4); }
void __cdecl IntPair_SouthEastAtExit() { markDestroyed(0x8); }
void __cdecl IntPair_SouthAtExit() { markDestroyed(0x10); }
void __cdecl IntPair_SouthWestAtExit() { markDestroyed(0x20); }
void __cdecl IntPair_WestAtExit() { markDestroyed(0x40); }
void __cdecl IntPair_NorthWestAtExit() { markDestroyed(0x80); }
}
