#include "nav.h"

#include <cstdint>
#include <initializer_list>

#include "../core/accessors.h"
#include "../core/crt.h"
#include "../core/savegame.h"
#include "../core/trivial.h"
#include "../math/intpair.h"
#include "../math/x87.h"
#include "mapcell.h"
#include "route.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

// a * b in 16.16 fixed point: the 64-bit product's bits 16-47 (imul; shl edx, 16; shr eax, 16; or).
s32 MulFixed(s32 a, s32 b) { return (s32)(((long long)a * b) >> 16); }

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

// The octagonal length of a - b (IntPair_Sub): the longer distance and half the shorter, as the larger plus the smaller less half the
// smaller (NEG for the distances: INT_MIN stays negative, sic).
s32 OctDist(const s32 *a, const s32 *b) {
    s32 d[2];
    IntPair_Sub(reinterpret_cast<const u32 *>(a), nullptr, reinterpret_cast<u32 *>(d), reinterpret_cast<const u32 *>(b));
    const s32 ax = d[0] < 0 ? (s32)(0u - (u32)d[0]) : d[0];
    const s32 ay = d[1] < 0 ? (s32)(0u - (u32)d[1]) : d[1];
    const s32 shorter = ax > ay ? ay : ax;
    return (s32)((u32)ay - (u32)(shorter >> 1) + (u32)ax);
}

// The centre (16.16) of the k-th cell of the route at 0x7cf108.
void CellCentre(s32 *out, s32 k) {
    out[0] = (s32)(((u32)global<u8>(0x007cf10e + k * 2) << 16) + 0x8000);
    out[1] = (s32)(((u32)global<u8>(0x007cf10f + k * 2) << 16) + 0x8000);
}

}  // namespace

namespace {

// The words cleared before the elements are built (the others written then are overwritten later).
const u32 kZeroed[] = {0x8, 0xc, 0x10, 0x14, 0x20, 0x24, 0x28, 0x2c, 0x30, 0x34, 0x38, 0x3c, 0x40, 0x44, 0x4c, 0x50, 0x54, 0x58, 0x5c};

}  // namespace

extern "C" {

void __fastcall Nav_SetPosVel(u8 *nav, void *, u32 x, u32 y, u32 vx, u32 vy) {
    at<u32>(nav, 8) = x;
    at<u32>(nav, 0xc) = y;
    at<u32>(nav, 0x10) = vx;
    at<u32>(nav, 0x14) = vy;
}

void __fastcall Nav_GetPos(const u8 *nav, void *, u32 *x, u32 *y) {
    *x = at<u32>(nav, 8);
    *y = at<u32>(nav, 0xc);
}
u8 *__fastcall Nav_Ctor(u8 *object) {
    at<u32>(object, 0) = 1;
    at<u32>(object, 4) = 0xa;
    for (u32 offset : kZeroed)
        at<u32>(object, offset) = 0;
    at<u32>(object, 0x18) = 0x10000;
    at<u32>(object, 0x1c) = 0x50000;
    at<u8>(object, 0x60) = 1;
    CRT_VectorCtorIterator(object + 0x64, 8, 5, reinterpret_cast<void(__fastcall *)(void *)>(IntPair_Ctor), reinterpret_cast<void *>(0x005d1750));
    for (u32 offset = 0xa0; offset <= 0xb8; offset += 4)
        at<u32>(object, offset) = 0;
    at<u32>(object, 4) = 0x3333;
    at<u32>(object, 0) = 0x10000;
    at<u32>(object, 8) = 0;
    at<u32>(object, 0xc) = 0;
    at<u32>(object, 0x1c) = 0x3333;
    at<u32>(object, 0x18) = 0x6666;
    return object;
}

void __fastcall Nav_Dtor(u8 *object) { CRT_VectorDtorIterator(object + 0x64, 8, 5, reinterpret_cast<void(__fastcall *)(void *)>(&Stub_Ret)); }

void __fastcall Nav_DtorAtD4(u8 *owner) { Nav_Dtor(owner + 0xd4); }

s32 *__fastcall Nav_WorldToLocal(const u8 *nav, void *, s32 *out, const s32 *point) {
    s32 d[2];
    IntPair_Sub(reinterpret_cast<const u32 *>(point), nullptr, reinterpret_cast<u32 *>(d), &at<u32>(nav, 8));
    const s32 yx = MulFixed(d[0], at<s32>(nav, 0x28)), yy = MulFixed(d[1], at<s32>(nav, 0x2c));
    const s32 xx = MulFixed(d[0], at<s32>(nav, 0x20)), xy = MulFixed(d[1], at<s32>(nav, 0x24));
    out[1] = (s32)((u32)yy + (u32)yx);
    out[0] = (s32)((u32)xy + (u32)xx);
    return out;
}

s32 *__fastcall Nav_PredictPos(const u8 *nav, void *, s32 *out, s32 time) {
    s32 step[2], ahead[2];
    IntPair_MulFixed(&at<s32>(nav, 0x10), nullptr, step, time);
    IntPair_Add(&at<u32>(nav, 8), nullptr, reinterpret_cast<u32 *>(ahead), reinterpret_cast<const u32 *>(step));
    out[0] = ahead[0];
    out[1] = ahead[1];
    return out;
}

s32 __fastcall Nav_PathProgress(const u8 *nav) {
    if (at<u8>(nav, 0x60) == 1) return 0x10000;
    s32 d[2];
    IntPair_Sub(&at<u32>(nav, 0x64 + at<u32>(nav, 0x5c) * 8), nullptr, reinterpret_cast<u32 *>(d), &at<u32>(nav, 8));
    const s32 ax = d[0] < 0 ? (s32)(0u - (u32)d[0]) : d[0];  // sic: INT_MIN stays negative
    const s32 ay = d[1] < 0 ? (s32)(0u - (u32)d[1]) : d[1];
    const s32 shorter = ax > ay ? ay : ax;
    const u32 left = at<u32>(nav, 0xa8) + at<u32>(nav, 0xa4) - (u32)(shorter >> 1) + (u32)ay + (u32)ax;
    const s32 total = at<s32>(nav, 0xa0);
    if (total == 0) return 0x10000;
    // left << 16 (as 64 bits) / total in one IDIV, as the original's: it faults (sic) when the quotient passes 31 bits.
    const u32 dividendLow = left << 16, dividendHigh = (u32)((s32)left >> 16);
    s32 q;
    __asm {
        mov eax, dividendLow
        mov edx, dividendHigh
        idiv total
        mov q, eax
    }
    return 0x10000 - q;
}

void __cdecl Nav_VisitNeighbours(const u8 *nav, s32 radius, void(__cdecl *visit)(const u8 *nav, const u8 *other)) {
    const s32 r8 = radius >> 8;
    const s32 reach2 = (s32)((u32)r8 * (u32)r8);
    const s32 cells = (s32)((u32)radius + 0xffff) >> 16;
    const s32 x = at<s32>(nav, 8), y = at<s32>(nav, 0xc);
    const s32 x8 = x >> 8, y8 = y >> 8;
    const s32 cx = x >> 16, cy = y >> 16;
    u32 visits = 0;
    for (s32 cellY = cy - cells; cellY <= cy + cells; ++cellY) {
        for (s32 cellX = cx - cells; cellX <= cx + cells; ++cellX) {
            if (!Map_InBounds(cellX, cellY)) continue;
            const u32 id = ((u32)cellY * 0x80 + (u32)cellX + 1) & 0xffff;
            const u8 *map = **reinterpret_cast<u8 ***>(0x7cf6ec);
            u16 handle = at<u16>(map, id * 0x44 + 0x2b8);
            while (handle) {
                const u8 *thing = *reinterpret_cast<u8 **>(0x7cfb90 + (u32)handle * 20);
                const u8 kind = thing[2];
                if (kind == 1 || kind == 4 || kind == 5 || kind == 6 || kind == 8 || kind == 7 || kind == 0x12) {
                    const u8 *other = thing + 0xd4;
                    if (nav != other) {
                        const u32 dx = (u32)(s32)at<s16>(thing, 4) - (u32)x8, dy = (u32)(s32)at<s16>(thing, 6) - (u32)y8;
                        const s32 d = (s32)(dx * dx + dy * dy);
                        if (d != 0 && d < reach2) {
                            visit(nav, other);
                            if (++visits >= 5) return;
                        }
                    }
                }
                handle = at<u16>(thing, 0xa);
            }
        }
    }
}
void __fastcall Nav_Update(u8 *nav) {
    typedef s32 *(__fastcall *Steer)(void *behaviour, void *, s32 *out, const u8 *nav);  // a behaviour's slot 0 (thiscall)
    typedef s32(__cdecl *Passable)(s32 from, s32 to, u32 flags);
    s32 sum[2] = {0, 0};
    for (u32 entry = *reinterpret_cast<u32 *>(0x007cf078); entry != *reinterpret_cast<u32 *>(0x007cf07c); entry += 8) {
        u8 *const behaviour = at<u8 *>(reinterpret_cast<void *>(entry), 4);
        s32 force[2], limited[2], weighted[2];
        (*reinterpret_cast<Steer *>(at<u32>(behaviour, 0)))(behaviour, nullptr, force, nav);
        IntPair_MulFixed(IntPair_Truncate(limited, force, at<s32>(nav, 0x18)), nullptr, weighted, at<s32>(reinterpret_cast<void *>(entry), 0));
        sum[0] += weighted[0];
        sum[1] += weighted[1];
    }
    s32 limited[2], accel[2], capped[2];
    u32 vel[2];
    IntPair_DivFixed(IntPair_Truncate(limited, sum, at<s32>(nav, 0x18)), nullptr, accel, 0x10000);
    IntPair_Truncate(capped, reinterpret_cast<s32 *>(IntPair_Add(&at<u32>(nav, 0x10), nullptr, vel, reinterpret_cast<u32 *>(accel))), at<s32>(nav, 0x1c));
    at<s32>(nav, 0x10) = capped[0];
    at<s32>(nav, 0x14) = capped[1];
    u32 next[2], from[2], to[2];
    IntPair_Add(&at<u32>(nav, 8), nullptr, next, &at<u32>(nav, 0x10));
    IntPair_FromFixed(from, at<u32>(nav, 8), at<u32>(nav, 0xc));
    IntPair_FromFixed(to, next[0], next[1]);
    const s32 toId = Map_IdAt((s32)to[0], (s32)to[1]);
    const s32 fromId = Map_IdAt((s32)from[0], (s32)from[1]);
    if ((*reinterpret_cast<Passable *>(0x00762190))(fromId, toId, at<u32>(nav, 0xb4)) == 0) {
        at<u32>(nav, 0xb0) = (at<u32>(nav, 0xb0) << 1) | 1;  // blocked: the position is kept
    } else {
        at<u32>(nav, 8) = next[0];
        at<u32>(nav, 0xc) = next[1];
    }
    s32 heading[2];
    u32 side[2];
    IntPair_Copy(&at<u32>(nav, 0x20), nullptr, reinterpret_cast<u32 *>(IntPair_Normalize(heading, &at<s32>(nav, 0x10))));
    IntPair_Copy(&at<u32>(nav, 0x28), nullptr, IntPair_Perp(side, &at<u32>(nav, 0x20)));
    const s32 progress = Nav_PathProgress(nav);
    u32 history = at<u32>(nav, 0xb0) << 1;
    at<u32>(nav, 0xb0) = history;
    if (progress <= at<s32>(nav, 0xac))
        at<u32>(nav, 0xb0) = history | 1;
    at<s32>(nav, 0xac) = progress;
}

u32 __fastcall Nav_Serialize(u8 *nav, void *, Stream *stream, u32 save, s32 version, u32) {
    static const SaveSpec kFields[] = {{0x40, 8, 0x0076208c}, {0x38, 8, 0x0076207c}, {0x20, 8, 0x00762070}, {0x28, 8, 0x00762064}, {0x0, 4, 0x0076205c},
                                       {0x18, 4, 0x00762050}, {0x1c, 4, 0x00762044}, {0xb8, 4, 0x00762034}, {0xb4, 4, 0x00762028}, {0x58, 4, 0x00762014},
                                       {0x5c, 4, 0x00762008}, {0x60, 1, 0x00761ff8}, {0xac, 4, 0x00761fe4}, {0xb0, 4, 0x00761fd0}, {0xa4, 4, 0x00761fbc},
                                       {0xa8, 4, 0x00761fac}, {0x4c, 8, 0x00761f9c}, {0x48, 4, 0x00761f8c}, {0x54, 4, 0x00761f78}, {0xa0, 4, 0x00761f68},
                                       {0x8, 8, 0x00761f64},  {0x4, 4, 0x00761f5c}};
    if (version < 0 || version > 1)
        return 0;
    if (!Save_Fields(stream, save, nav, kFields, 2))
        return 1;
    if (save == 0)
        at<u32>(nav, 0x34) = 0;
    if (!Save_Fields(stream, save, nav, kFields + 2, 20))
        return 1;
    for (u32 i = 0; i < 5; ++i)
        if (!Save_Field(stream, save, nav + 0x64 + i * 8, 8, 0x00761f48) || !Save_Field(stream, save, nav + 0x8c + i * 4, 4, 0x00761f38))
            return 1;
    if (save == 0)
        at<u32>(nav, 0x30) = 0;
    return Save_Field(stream, save, nav + 0x10, 8, 0x00761f34) ? 0 : 1;
}

u32 __fastcall Nav_SerializeFwd(u8 *nav, void *, Stream *stream, u32 save, s32 version, u32 unused) {
    return Nav_Serialize(nav, nullptr, stream, save, version, unused);
}

u32 __fastcall Nav_PathTo(u8 *nav, void *, const s32 *target, u32 prepend) {
    u8 *const route = &global<u8>(0x007cf108);
    route[2] = (u8)(at<s32>(nav, 8) >> 16);
    route[3] = (u8)(at<s32>(nav, 0xc) >> 16);
    route[4] = (u8)(target[0] >> 16);
    route[5] = (u8)(target[1] >> 16);
    if (Route_Find(route, at<u32>(nav, 0xb4)) == 0x70000000) {
        at<s32>(nav, 0x4c) = target[0];
        at<s32>(nav, 0x50) = target[1];
        for (u32 offset : {0x54u, 0x58u, 0xa0u, 0xa4u, 0xa8u, 0xacu, 0xb0u, 0x5cu})
            at<u32>(nav, offset) = 0;
        at<u8>(nav, 0x60) = 1;
        at<u32>(nav, 0xb8) = 1;
        return 0;
    }
    if (prepend == 1) {  // the nav's own cell put first (the cells moved up one, the last of 0xfc dropped)
        for (u32 cell = 0x007cf304; (s32)cell > 0x007cf10e; cell -= 2)
            global<u16>(cell) = global<u16>(cell - 2);
        route[6] = (u8)(at<s32>(nav, 8) >> 16);
        route[7] = (u8)(at<s32>(nav, 0xc) >> 16);
        if (route[0] < 0xfc)
            ++route[0];
    }
    at<s32>(nav, 0x4c) = target[0];
    at<s32>(nav, 0x50) = target[1];
    at<s32>(nav, 0x54) = route[0];
    at<s32>(nav, 0x58) = route[0] < 5 ? route[0] : 5;
    s32 a[2], b[2];
    CellCentre(a, 0);
    at<s32>(nav, 0xa0) = OctDist(a, &at<s32>(nav, 8));
    for (s32 k = 0; k < (s32)route[0] - 1; ++k) {
        CellCentre(a, k);
        CellCentre(b, k + 1);
        at<u32>(nav, 0xa0) += (u32)OctDist(b, a);
    }
    for (s32 k = 0; k < at<s32>(nav, 0x58); ++k)
        CellCentre(&at<s32>(nav, 0x64 + k * 8), k);
    at<u32>(nav, 0xa4) = 0;
    for (s32 k = 0; k < at<s32>(nav, 0x58) - 1; ++k) {
        const s32 d = OctDist(&at<s32>(nav, 0x64 + (k + 1) * 8), &at<s32>(nav, 0x64 + k * 8));
        at<s32>(nav, 0x8c + k * 4) = d;
        at<u32>(nav, 0xa4) += (u32)d;
    }
    at<u32>(nav, 0xa8) = 0;
    for (s32 k = at<s32>(nav, 0x58) - 1; k < at<s32>(nav, 0x54) - 1; ++k) {
        CellCentre(a, k);
        CellCentre(b, k + 1);
        at<u32>(nav, 0xa8) += (u32)OctDist(b, a);
    }
    // (the original also takes the first path point less the position, unused)
    at<u32>(nav, 0xac) = 0;
    at<u32>(nav, 0xb0) = 0;
    at<u32>(nav, 0x5c) = 0;
    at<u8>(nav, 0x60) = 0;
    at<u32>(nav, 0xb8) = 0;
    at<u32>(nav, 0x48) = Counter7cdb98_Inc();
    return 1;
}

u32 __fastcall Nav_PathToPoint(u8 *nav, void *, s32 x, s32 y) {
    const s32 target[2] = {x, y};
    return Nav_PathTo(nav, nullptr, target, 0);
}

void __fastcall Nav_SetSpeed(u8 *nav, void *, float speed) {
    if (speed > 2.0f)
        speed = 2.0f;
    // fld speed; fmul a double; __ftol's low dword: max_force (+0x18) and max_speed (+0x1c), each at least 0x28f (0.01)
    const s32 force = (s32)(long long)X87_MulFloatDouble(speed, 26214.4);
    at<s32>(nav, 0x18) = force;
    const s32 top = (s32)(long long)X87_MulFloatDouble(speed, 13107.2);
    at<s32>(nav, 0x1c) = top;
    if (force < 0x28f)
        at<s32>(nav, 0x18) = 0x28f;
    if (top < 0x28f)
        at<s32>(nav, 0x1c) = 0x28f;
}
}
