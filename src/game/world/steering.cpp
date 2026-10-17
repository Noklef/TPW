#include "steering.h"

#include <cstdint>
#include <cstring>

#include "../math/intpair.h"
#include "../math/matrix.h"
#include "nav.h"

namespace {

u32 *Construct(u32 *self, u32 vtable, const char *name) {
    self[0] = vtable;
    std::strcpy(reinterpret_cast<char *>(self + 1), name);  // repne scasb; rep movsd; rep movsb: the name and its 0
    return self;
}

u32 *Global(u32 address) { return reinterpret_cast<u32 *>(address); }

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

s32 &g_force(u32 i) { return reinterpret_cast<s32 *>(0x7cf100)[i]; }  // the summed force, x and y
s32 &g_nearestHit() { return *reinterpret_cast<s32 *>(0x7cf32c); }     // avoid_obstacle's nearest hit so far
s32 &g_lookAhead() { return *reinterpret_cast<s32 *>(0x7cf048); }      // avoid_obstacle's reach

// EDX:EAX / divisor in one IDIV, as the originals: it faults (sic) on 0 or a quotient past 31 bits.
s32 Idiv(u32 dividendLow, u32 dividendHigh, s32 divisor) {
    s32 q;
    __asm {
        mov eax, dividendLow
        mov edx, dividendHigh
        idiv divisor
        mov q, eax
    }
    return q;
}

// a << 16 / divisor (the 16.16 quotient: shl eax, 16; sar edx, 16; idiv).
s32 DivFixed(s32 a, s32 divisor) { return Idiv((u32)a << 16, (u32)(a >> 16), divisor); }

// a * b in 16.16 fixed point: the 64-bit product's bits 16-47.
s32 MulFixed(s32 a, s32 b) { return (s32)(((long long)a * b) >> 16); }

s32 Abs(s32 v) { return v < 0 ? (s32)(0u - (u32)v) : v; }  // sic: INT_MIN stays negative

// The length of a 16.16 pair the behaviours inline: Math_ISqrt of x * x + y * y (32-bit products) when both magnitudes
// are under 0x1000, else of the magnitudes shifted right by 8 (under 0x400000) or 18 (logically), shifted back.
u32 Length(const s32 *d) {
    const u32 ax = (u32)Abs(d[0]), ay = (u32)Abs(d[1]);
    const u32 both = ay | ax;
    if (!(both & 0xfffff000)) return Math_ISqrt(ax * ax + ay * ay);
    if (!(both & 0xffc00000)) return Math_ISqrt((ax >> 8) * (ax >> 8) + (ay >> 8) * (ay >> 8)) << 8;
    return Math_ISqrt((ax >> 18) * (ax >> 18) + (ay >> 18) * (ay >> 18)) << 18;
}

u32 *U(s32 *p) { return reinterpret_cast<u32 *>(p); }

// The octagonal length the callbacks inline: |x| + |y| - min(|x|, |y|) / 2 (arithmetic shift, 32-bit sums).
s32 Octagonal(const s32 *d) {
    const s32 ax = Abs(d[0]), ay = Abs(d[1]);
    const s32 shorter = ax > ay ? ay : ax;
    return (s32)((u32)ay - (u32)(shorter >> 1) + (u32)ax);
}

}  // namespace

extern "C" {

u32 *__fastcall SteerSeek_Ctor(u32 *self) { return Construct(self, 0x7009b8, "seek"); }
u32 *__fastcall SteerArrival_Ctor(u32 *self) { return Construct(self, 0x7009c0, "arrival"); }
u32 *__fastcall SteerSeparation_Ctor(u32 *self) { return Construct(self, 0x7009c4, "separation"); }
u32 *__fastcall SteerAvoidObstacle_Ctor(u32 *self) { return Construct(self, 0x7009c8, "avoid_obstacle"); }
u32 *__fastcall SteerRepelObstacle_Ctor(u32 *self) { return Construct(self, 0x7009cc, "repel_obstacle"); }
u32 *__fastcall SteerAvoidWalls_Ctor(u32 *self) { return Construct(self, 0x7009d0, "avoid_walls"); }
u32 *__fastcall SteerFollowPath_Ctor(u32 *self) { return Construct(self, 0x7009d4, "follow_path"); }

void __fastcall Steer_Dtor(u32 *self) { self[0] = 0x7009bc; }

void __cdecl SteerSeek_Destroy() { Steer_Dtor(Global(0x7cf050)); }
void __cdecl SteerArrival_Destroy() { Steer_Dtor(Global(0x7cf0b0)); }
void __cdecl SteerSeparation_Destroy() { Steer_Dtor(Global(0x7cf0d8)); }
void __cdecl SteerAvoidObstacle_Destroy() { Steer_Dtor(Global(0x7cf360)); }
void __cdecl SteerRepelObstacle_Destroy() { Steer_Dtor(Global(0x7cf338)); }
void __cdecl SteerAvoidWalls_Destroy() { Steer_Dtor(Global(0x7cf088)); }
void __cdecl SteerFollowPath_Destroy() { Steer_Dtor(Global(0x7cf308)); }

s32 *__fastcall SteerSeek_Steer(void *, void *, s32 *out, const u8 *nav) {
    s32 toTarget[2];
    IntPair_Sub(at<const u32 *>(nav, 0x30), nullptr, U(toTarget), &at<u32>(nav, 8));
    IntPair_Sub(U(toTarget), nullptr, U(out), &at<u32>(nav, 0x10));
    return out;
}

s32 *__fastcall SteerArrival_Steer(void *, void *, s32 *out, const u8 *nav) {
    const s32 slowing = 0x20000;  // __ftol(2.0 * 65536.0)
    s32 toTarget[2];
    IntPair_Sub(at<const u32 *>(nav, 0x30), nullptr, U(toTarget), &at<u32>(nav, 8));
    const s32 distance = (s32)Length(toTarget);
    if (distance == 0) {
        out[0] = 0;
        out[1] = 0;
        return out;
    }
    const s32 maxSpeed = at<s32>(nav, 0x1c);
    const s32 speed = maxSpeed < DivFixed(distance, slowing) ? maxSpeed : DivFixed(distance, slowing);
    s32 desired[2];
    IntPair_MulFixed(toTarget, nullptr, desired, DivFixed(speed, distance));
    IntPair_Sub(U(desired), nullptr, U(out), &at<u32>(nav, 0x10));
    return out;
}

void __cdecl SteerRepel_Neighbour(const u8 *nav, const u8 *other) {
    s32 away[2];
    IntPair_Sub(&at<u32>(nav, 8), nullptr, U(away), reinterpret_cast<const u32 *>(other));
    const s32 ax = Abs(away[0]), ay = Abs(away[1]);
    const s32 shorter = ax > ay ? ay : ax;
    const s32 distance = (s32)((u32)ay - (u32)(shorter >> 1) + (u32)ax);
    if (distance >= (s32)(at<u32>(nav, 4) + at<u32>(other, 8) + 0x10000)) return;
    const s32 root = (s32)Math_ISqrt((u32)distance);
    s32 push[2];
    IntPair_MulFixed(away, nullptr, push, Idiv(0, 1, root));  // 0x10000 << 16 / root: 2^32 / root (it faults, sic, for roots 0 to 2)
    g_force(0) = (s32)((u32)g_force(0) + (u32)push[0]);
    g_force(1) = (s32)((u32)g_force(1) + (u32)push[1]);
}

s32 __cdecl Steer_CircleHit(const s32 *local, s32 radius) {
    const s32 y = local[1] >> 18, twiceX = (s32)(0u - (u32)local[0] * 2), x = local[0] >> 18;
    const s32 r2 = MulFixed(radius, radius), x2 = MulFixed(twiceX, twiceX);
    const s32 disc = (s32)((u32)r2 * 4 - (((u32)x * (u32)x) << 2) - (((u32)y * (u32)y) << 2) + (u32)x2);
    if (disc <= 0) return (s32)0x80000000;
    return (s32)((0u - (Math_ISqrt((u32)disc) + (u32)twiceX)) >> 1);
}

s32 __cdecl Steer_RayCircleHit(const s32 *from, const s32 *dir, const s32 *centre, s32 radius) {
    s32 d[2];
    IntPair_Sub(reinterpret_cast<const u32 *>(from), nullptr, U(d), reinterpret_cast<const u32 *>(centre));
    const s32 a = (s32)((u32)MulFixed(dir[1], dir[1]) + (u32)MulFixed(dir[0], dir[0]));
    const s32 b = (s32)(((u32)MulFixed(dir[0], d[0]) + (u32)MulFixed(dir[1], d[1])) << 1);
    const s32 c = (s32)((u32)MulFixed(d[1], d[1]) + (u32)MulFixed(d[0], d[0]) - (u32)MulFixed(radius, radius));
    const s32 disc = (s32)((u32)MulFixed(b, b) - ((u32)MulFixed(a, c) << 2));
    if (disc <= 0) return (s32)0x80000000;
    const u32 root = Math_ISqrt((u32)disc);
    const s32 nearer = (s32)(0u - (root + (u32)b)), farther = (s32)(root - (u32)b);
    return (nearer <= farther ? nearer : farther) >> 1;
}

void __cdecl SteerAvoidObstacle_Obstacle(const u8 *nav, const s32 *obstacle) {
    s32 local[2];
    Nav_WorldToLocal(nav, nullptr, local, obstacle);
    if (local[0] <= 0) return;
    const s32 radius = obstacle[2];
    if ((s32)((u32)local[0] - (u32)radius) > g_lookAhead()) return;
    if (Abs(local[1]) >= (s32)(at<u32>(nav, 4) + (u32)radius)) return;
    const s32 hit = Steer_CircleHit(local, radius);
    if (hit < 0 || hit >= g_nearestHit()) return;
    g_nearestHit() = hit;
    g_force(1) = (s32)(0u - (u32)local[1]);
}

s32 *__fastcall SteerAvoidObstacle_Steer(void *, void *, s32 *out, const u8 *nav) {
    g_force(0) = 0;
    g_force(1) = 0;
    g_nearestHit() = 0x7fffffff;
    s32 ahead[2], reach[2];
    Nav_PredictPos(nav, nullptr, ahead, 0xa0000);
    IntPair_Sub(U(ahead), nullptr, U(reach), &at<u32>(nav, 8));
    g_lookAhead() = (s32)(at<u32>(nav, 4) + Length(reach));
    // (the obstacles were visited here, through SteerAvoidObstacle_Obstacle, in a call the retail build compiles out)
    if (g_nearestHit() == 0x7fffffff) {
        out[0] = 0;
        out[1] = 0;
        return out;
    }
    Matrix2_Transform(const_cast<u8 *>(nav), nullptr, out, &g_force(0));
    return out;
}

void __cdecl SteerSeparation_Neighbour(const u8 *nav, const u8 *other) {
    if (other == nav) return;
    s32 away[2];
    IntPair_Sub(&at<u32>(nav, 8), nullptr, U(away), &at<u32>(other, 8));
    const s32 distance = Octagonal(away);
    if (distance >= (s32)(at<u32>(other, 4) + at<u32>(nav, 4) + 0x10000) || distance == 0) return;
    s32 push[2];
    IntPair_DivFixed(away, nullptr, push, distance);
    g_force(0) = (s32)((u32)g_force(0) + (u32)push[0]);
    g_force(1) = (s32)((u32)g_force(1) + (u32)push[1]);
}

s32 *__fastcall SteerSeparation_Steer(void *, void *, s32 *out, const u8 *nav) {
    const u32 point = at<u32>(nav, 0x5c);
    if (point == at<u32>(nav, 0x54) - 1 || at<u8>(nav, 0x60) != 0) {
        s32 d[2];
        IntPair_Sub(&at<u32>(nav, 8), nullptr, U(d), &at<u32>(nav, 0x64 + point * 8));
        if (Octagonal(d) <= (s32)(at<u32>(nav, 4) + 0x10000)) {
            out[0] = 0;
            out[1] = 0;
            return out;
        }
    }
    g_force(0) = 0;
    g_force(1) = 0;
    Nav_VisitNeighbours(nav, (s32)(at<u32>(nav, 4) << 1), &SteerSeparation_Neighbour);
    out[0] = g_force(0);
    out[1] = g_force(1);
    return out;
}
}
