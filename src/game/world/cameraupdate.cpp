#include "cameraupdate.h"

#include <cmath>
#include <cstdint>
#include <cstring>

#include "../core/gameclock.h"
#include "../core/keymap.h"
#include "../math/mat4.h"
#include "../math/matrix.h"
#include "../math/x87.h"
#include "../terrain/land.h"
#include "camera.h"
#include "mapcell.h"
#include "object.h"
#include "objectdefs.h"
#include "../ui/windowproc.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

u16 __fastcall IsHeld(const u8 *map, void *, u32 action) { return KeyMap_IsHeld(map, nullptr, (s16)action); }
u8 *__fastcall FindDef(u8 *defs, void *, u32 id) { return ObjectDefs_Find(defs, nullptr, (u16)id); }
double __cdecl Height(u32 x, u32 z, u32 any) {
    float fx, fz;
    std::memcpy(&fx, &x, 4);
    std::memcpy(&fz, &z, 4);
    return Terrain_GridHeightAt(fx, fz, any);
}
const CameraUpdateEnv kGameEnv = {&Clocks_RealNow, &IsHeld, &MapCell_StepBlocked, &Cell_FindRideableObject, &FindDef, &Object_FollowCamera, &Height, &Camera_MatricesClose};
const CameraUpdateEnv *gEnv = &kGameEnv;

const float kOne = 1.0f;                 // 0x6fdd54
const float kTenth = 0.100000001f;       // 0x6fdd58
const float kMinusTen = -10.0f;          // 0x6fdd5c
const float kTen = 10.0f;                // 0x6fdd7c
const float kHalfPi = 1.57079637f;       // 0x6fdd68
const float kMinusHalfPi = -1.57079637f; // 0x6fdd94
const float kMinusQuarterPi = -0.785398185f;   // 0x6fdd90
const float kQuarterPi = 0.785398185f;   // 0x6fdd98
const float kNearTen = 9.9989996f;       // 0x74c9c4
const float kNudge = 0.00100000005f;     // 0x74c9d0
const float kTieSpeed = 1.00999999f;     // 0x74c9c8
const float kDecay = -0.00999999978f;    // 0x6fddc8
const float kTiltStep = -0.100000001f;   // 0x6fddac
const float kEdge = 0.949999988f;        // 0x6fddc4
const float kMinusEdge = -0.949999988f;  // 0x6fddc0
const float kInner = 0.200000003f;       // 0x6fdde0
const float kMinusInner = -0.200000003f; // 0x6fddd8
const float kOuter = 0.400000006f;       // 0x6fddec
const float kMinusOuter = -0.400000006f; // 0x6fdde4
const float kBlendRate = 0.00499999989f; // 0x6fddcc
const float kBlendSlow = 0.00714285718f; // 0x6fddd4
const float kSmall = 9.99999975e-05f;    // 0x6fde04 (0x74c988 the same)
const float kMinusSmall = -9.99999975e-05f;   // 0x6fde00
const float kTurnClose = 0.000500000024f;     // 0x74c984
const double kHalfPiD = 1.5707963705062866;   // 0x6fdd88
const double kTwoPiD = 6.2831854820251465;    // 0x6fde38
const double kHundred = 100.0;           // 0x6fde20

inline u32 Bits(float x) {
    u32 b;
    std::memcpy(&b, &x, 4);
    return b;
}

inline bool Below0(float x) { return !(x >= 0.0f); }   // (or a NaN)
inline bool Zero(double x) { return !(x < 0.0 || x > 0.0); }   // (or a NaN)
inline s32 Cell(float x) { return (s32)(long long)(X87Reg(x) * kTenth).v; }   // __ftol
inline u16 Held(u32 action) { return gEnv->isHeld(global<const u8 *>(0x00787c14), nullptr, action); }

// A register's product with a double one (fsin, fcos): rounded once.
inline X87Reg Mul(double a, double b) { return a != a && b != b ? X87Reg(X87_Add(a, b)) : X87Reg(X87_MulDouble(a, b)); }

// The angle 0x7909e0 eased to the yaw (a register); within 0.0005 of it, the yaw.
void EaseAngle(X87Reg yaw, float dt, u32 paused) {
    float &angle = global<float>(0x007909e0);
    const float limit = paused ? 200.0f : 140.0f;
    const X87Reg s = X87Reg(dt < limit ? dt : limit) * (paused ? kBlendRate : kBlendSlow);
    const X87Reg sum = (X87Reg(kOne) - s) * angle + yaw * s;
    angle = (float)sum;
    if (!(std::fabs((sum - yaw).v) >= kTurnClose)) angle = (float)yaw;
}

// A coordinate at the edge of cell n it leaves, by the sign of what is left to go.
inline float CellEdge(u32 n, float left) {
    if (Below0(left)) return (float)X87Reg(X87_MulInt(n, kTen));
    return (float)(kNearTen - X87Reg(X87_MulInt(n, kMinusTen)));
}

// The fraction left of the cell, over the step; *slot keeps the fraction when the step goes up.
float CellShare(float at_, float step, float scaled, u32 &slot) {
    const X87Reg place = X87Reg(at_) * kTenth;
    double whole;
    const double fraction = std::modf(place.v, &whole);   // CRT modf (0x67b280)
    if (Below0(step)) return (float)std::fabs((X87Reg(fraction) / scaled).v);
    const float f = (float)fraction;
    slot = Bits(f);
    return (float)std::fabs(((X87Reg(kOne) - f) / scaled).v);
}

// The point moved along its way one cell at a time. Whether it went on the way at all.
bool WalkPoint(float dt, u32 &s1c, u32 &s2c, u32 &s40) {
    float &x = global<float>(0x007908f0);
    float &z = global<float>(0x007908f8);
    const X87Reg dx = X87Reg(dt) * global<float>(0x00790a88);
    float left = (float)dx;
    float leftZ = (float)(X87Reg(dt) * global<float>(0x00790a90));
    if (dx.v >= kMinusSmall && !(left > kSmall)) left = 0.0f;
    if (leftZ >= kMinusSmall && !(leftZ > kSmall)) leftZ = 0.0f;
    if (Zero(left) && Zero(leftZ)) return false;
    u32 more;
    do {
        const u32 cx = (u32)Cell(x), cz = (u32)Cell(z);
        const float scaledX = (float)(X87Reg(left) * kTenth), scaledZ = (float)(X87Reg(leftZ) * kTenth);
        float tx = 2.0f, tz = 2.0f;
        if (!Zero(left)) tx = CellShare(x, left, scaledX, s1c);
        if (!Zero(leftZ)) tz = CellShare(z, leftZ, scaledZ, s1c);
        if (!(tx < tz || tx > tz) && !Zero(left) && !Zero(leftZ)) {   // (a tie or a NaN: x reaches its edge first)
            const X87Reg l = X87Reg(left) / kTieSpeed, t = X87Reg(tx) * kTieSpeed;
            left = (float)l;
            tx = (float)t;
        }
        float px, pz;
        if (!(tx >= tz) && !(tx >= kOne)) {   // x first
            s2c = (s2c & 0xffffff00) | (Below0(left) ? 3 : 1);   // (a stale slot's upper bytes pushed with it, sic)
            X87Reg rest = 0.0;
            if (gEnv->stepBlocked(cx, cz, s2c, 2)) {
                px = CellEdge(cx, left);
                left = 0.0f;
            } else {
                const X87Reg d = X87Reg(tx) * left;
                px = (float)(X87Reg(x) + d);
                rest = X87Reg(left) - d;
                left = (float)rest;
                if ((u32)Cell(px) == cx) px = !(rest.v >= 0.0) ? (float)(X87Reg(px) - kNudge) : (float)(X87Reg(px) + kNudge);
            }
            const X87Reg d = X87Reg(tx) * leftZ;
            pz = (float)(X87Reg(z) + d);
            leftZ = (float)(X87Reg(leftZ) - d);
            if ((u32)Cell(pz) != cz) pz = CellEdge(cz, leftZ);
            more = !(Zero(rest.v) && Zero(leftZ));
        } else if (tx >= tz && !(tz >= kOne)) {   // z first
            s2c = (s2c & 0xffffff00) | (Below0(leftZ) ? 0 : 2);
            if (gEnv->stepBlocked(cx, cz, s2c, 2)) {
                pz = CellEdge(cz, leftZ);
                leftZ = 0.0f;
            } else {
                const X87Reg d = X87Reg(tz) * leftZ;
                pz = (float)(X87Reg(z) + d);
                leftZ = (float)(X87Reg(leftZ) - d);
                if ((u32)Cell(pz) == cz) pz = Below0(leftZ) ? (float)(X87Reg(pz) - kNudge) : (float)(X87Reg(pz) + kNudge);
            }
            const X87Reg d = X87Reg(tz) * left;
            px = (float)(X87Reg(x) + d);
            left = (float)(X87Reg(left) - d);
            if ((u32)Cell(px) != cx) px = CellEdge(cx, left);
            more = !(Zero(left) && Zero(leftZ));
        } else {   // all the way, within the cell or to its edges
            px = (float)(X87Reg(left) + x);
            pz = (float)(X87Reg(leftZ) + z);
            if ((u32)Cell(px) != cx) px = CellEdge(cx, left);
            if ((u32)Cell(pz) != cz) pz = CellEdge(cz, leftZ);
            more = 0;
        }
        s32 cellX = Cell(px), cellZ = Cell(pz);
        Camera_ClampCell(&cellX, &cellZ);
        s1c = (u32)cellX;
        const u8 *const object = gEnv->findRideable(cellX, cellZ);
        if (object) {
            s40 = (s40 & 0xffff0000) | at<u16>(object, 0xe);   // (the word over what the slot held, sic)
            gEnv->findDef(reinterpret_cast<u8 *>((uintptr_t)0x007890a0), nullptr, s40);
            gEnv->follow(object, nullptr, 1);
            more = 0;
        }
        x = px;
        global<u32>(0x007908f4) = 0;
        z = pz;
    } while (more);
    return true;
}

}  // namespace

extern "C" {

void CameraUpdate_SetEnv(const CameraUpdateEnv *env) { gEnv = env ? env : &kGameEnv; }

void __cdecl Camera_Update() {
    float *const matrix = reinterpret_cast<float *>((uintptr_t)0x00790990);
    if (global<u8>(0x00790ab0) & 0x80) {
        global<float *>(0x00790a98) = reinterpret_cast<float *>((uintptr_t)0x007909f0);
        Mat4_Assign(global<u32 *>(0x0087afe8), nullptr, reinterpret_cast<const u32 *>((uintptr_t)0x007909f0));
    } else {
        float &yaw = global<float>(0x00790a38);
        float &angle = global<float>(0x007909e0);
        float &zoom = global<float>(0x007909ec);
        float &px = global<float>(0x007908f0);
        float &pz = global<float>(0x007908f8);
        float &vx = global<float>(0x00790a88);
        float &vy = global<float>(0x00790a8c);
        float &vz = global<float>(0x00790a90);
        u32 &keys = global<u32>(0x00790aac);
        u32 &flagsNow = global<u32>(0x00790ab0);
        const u32 paused = global<u8>(0x0078d912);
        u32 s2c = 0;   // (frame slots read again later, stale)
        const u32 now = gEnv->now(reinterpret_cast<u8 *>((uintptr_t)0x00785970));
        float dt;
        if (global<u32>(0x007908e8)) {
            if (paused) {
                const X87Reg r = std::fmod((double)yaw, kHalfPiD);   // _CIfmod (0x67b24a), exact
                if (!(r.v >= 0.0)) {
                    if (!(r.v >= kMinusQuarterPi))
                        yaw = (float)(yaw - (r - kMinusHalfPi));
                    else
                        yaw = (float)(X87Reg(yaw) - r);
                } else if (r.v > kQuarterPi) {
                    yaw = (float)((X87Reg(kHalfPi) - r) + yaw);
                } else {
                    yaw = (float)(X87Reg(yaw) - r);
                }
            }
            dt = 0.0f;
            global<u32>(0x007908e8) = 0;
        } else {
            dt = (float)(X87Reg((double)(u32)(now + 1)) - X87Reg((double)global<s32>(0x007909e4)));
        }
        if (dt > 200.0f) dt = 100.0f;
        if (!(dt > 0.0f)) dt = 1.0f;

        const u32 before = global<u32>(0x007909d0);
        if (before != keys && !(before & 4) && (keys & 4)) {
            global<u32>(0x007909d8) = global<u32>(0x00790ac8);
            global<u32>(0x007909dc) = global<u32>(0x00790acc);
        }
        keys &= 0xffffff87;
        if (Held(6) || Held(0x12)) keys |= 8;
        if (Held(7) || Held(0x13)) keys |= 0x10;
        if (Held(0) || Held(0x15)) keys |= 0x20;
        if (Held(1) || Held(0x16)) keys |= 0x40;
        u32 k = keys;
        u32 flags = flagsNow;
        float s28 = 0.0f;
        if (flags & 0x16) {
            if (!(flags & 0x3c)) {
                if (k & 8) yaw = (float)(yaw - X87Reg(dt) * 0.00120000006f);
                if (k & 0x10) yaw = (float)(yaw - X87Reg(dt) * -0.00120000006f);
                if (k & 0x40) s28 = kTiltStep;
                if (k & 0x20) s28 = (float)(X87Reg(s28) - kTiltStep);
            }
        } else {
            if (paused) {
                const u32 last = global<u8>(0x007909d0);
                if ((k & 8) && !(last & 8)) yaw = (float)(X87Reg(yaw) - kMinusHalfPi);
                if ((k & 0x10) && !(last & 0x10)) yaw = (float)(X87Reg(yaw) - kHalfPi);
            } else {
                if (k & 8) yaw = (float)(yaw - X87Reg(dt) * -0.00200000009f);
                if (k & 0x10) yaw = (float)(yaw - X87Reg(dt) * 0.00200000009f);
            }
            if (k & 0x40) zoom = (float)(zoom - X87Reg(dt) * -0.0500000007f);
            if (k & 0x20) zoom = (float)(zoom - X87Reg(dt) * 0.0500000007f);
        }
        if (k & 0x100) {
            global<u32>(0x007909ec) = global<u32>(0x006fdd40);
            global<u32>(0x00790a38) = global<u32>(0x006fdd44);
            global<u32>(0x007908f0) = global<u32>(0x006fdd48);
            global<u32>(0x007908f4) = global<u32>(0x006fdd4c);
            global<u32>(0x007908f8) = global<u32>(0x006fdd50);
            k &= ~0x100u;
            flags &= ~0x1000u;
            keys = k;
            flagsNow = flags;
        }
        float mx = (float)((X87Reg(global<float>(0x00790ac8)) - global<float>(0x007a1c54)) / global<float>(0x0074d104));   // (the screen's half size, set with the window)
        float my = (float)X87Reg(-((X87Reg(global<float>(0x00790acc)) - global<float>(0x007a1c58)) / global<float>(0x0074d108)).v);
        global<u32>(0x007909d0) = k;
        u32 pressing = 0, moved = 0;
        if (flags & 0x16) {
            moved = (~flags >> 0xb) & 1;
        } else {
            if ((reinterpret_cast<u8 &>(gWindowproc_7a1a14) & 8) && !(flags & 0x100) && (!(mx >= kMinusEdge) || mx > kEdge || !(my >= kMinusEdge) || my > kEdge)) pressing = 1;
            if (Held(0xd) || Held(0xe) || Held(0xf) || Held(0x10)) s2c = 1;
            flags = flagsNow;
            k = keys;
        }
        const u8 *const extent = at<const u8 *>(global<const u8 *>(0x007a0854), 0x6c);
        u32 s40 = at<u32>(extent, 0x1c);
        const X87Reg width = X87_MulInt(at<u32>(extent, 0x18), at<float>(extent, 0x10));
        const float depth = (float)X87Reg(X87_MulInt(at<u32>(extent, 0x1c), at<float>(extent, 0x14)));
        u32 s1c = Bits(depth);
        u32 edges = 0;
        if (Below0(px)) edges = 1;
        if (Below0(pz)) edges |= 4;
        if (px > width.v) edges |= 2;
        if (pz > depth) edges |= 8;

        const u32 grab = k & 4;
        if (!grab && !pressing && !moved && !s2c) {
            const X87Reg d = kOne - X87Reg(dt) * kDecay;
            const X87Reg y = X87Reg(yaw);
            const X87Reg nx = X87Reg(vx) / d, ny = X87Reg(vy) / d, nz = X87Reg(vz) / d;
            vx = (float)nx;
            vy = (float)ny;
            vz = (float)nz;
            EaseAngle(y, dt, paused);
        } else {
            X87Reg s = 0.0f;
            if (flags & 0x16) {
                const u32 follows = flags & 0x3c;
                if (follows) {
                    if (!(mx >= kMinusInner))
                        global<float>(0x00790ae0) = (float)((X87Reg(mx) - kMinusInner) * 3.5f);
                    else if (mx > kInner)
                        global<float>(0x00790ae0) = (float)((X87Reg(mx) - kInner) * 3.5f);
                    else
                        global<float>(0x00790ae0) = 0.0f;
                } else {
                    if (!(mx >= kMinusOuter)) {
                        const X87Reg t = X87Reg(mx) - kMinusOuter;
                        global<float>(0x00790ae0) = (float)(t + t);
                    } else if (mx > kOuter) {
                        const X87Reg t = X87Reg(mx) - kOuter;
                        global<float>(0x00790ae0) = (float)(t + t);
                    } else {
                        global<float>(0x00790ae0) = 0.0f;
                    }
                }
                if (!(my >= kMinusOuter))
                    global<float>(0x00790adc) = (float)(X87Reg(-(double)my) - kOuter);
                else if (my > kInner)
                    global<float>(0x00790adc) = (float)((X87Reg(kInner) - my) * 1.20000005f);
                else
                    global<float>(0x00790adc) = 0.0f;
                if (grab && !follows) s28 = (float)(X87Reg(s28) - kTiltStep);
            } else {
                const bool free = global<u8>(0x0078d910) == 0;
                if (grab && !free) {
                    flags &= ~0x1000u;
                    s = X87Reg(mx);
                    s28 = my;
                    flagsNow = flags;
                } else {
                    s28 = 0.0f;
                    if (!free) mx = my = 0.0f;
                    if (Held(0xd)) my = 1.0f;
                    if (Held(0xe)) my = -1.0f;
                    if (Held(0xf)) mx = -1.0f;
                    if (Held(0x10)) mx = 1.0f;
                    flags = flagsNow;
                    if (!(mx >= kMinusEdge) || mx > kEdge) {
                        const X87Reg t = !(mx >= kMinusEdge) ? X87Reg(mx) - kMinusEdge : X87Reg(mx) - kEdge;
                        flags &= ~0x1000u;
                        s = t * kTen;
                        flagsNow = flags;
                    }
                    if (!(my >= kMinusEdge) || my > kEdge) {
                        const X87Reg t = !(my >= kMinusEdge) ? X87Reg(my) - kMinusEdge : X87Reg(my) - kEdge;
                        s28 = (float)(t * kTen);
                        flags &= ~0x1000u;
                        flagsNow = flags;
                    }
                    k = keys;
                }
            }
            X87Reg p = 0.0, q = 0.0;
            if (flags & 0x16) {
                if (flags & 0x3c) {
                    global<u32>(0x007909e0) = global<u32>(0x00790a38);
                    q = X87Reg(s28);
                    p = s;
                } else {
                    const X87Reg r = X87Reg(dt) * global<float>(0x00790ae0) * -0.00120000006f;
                    const float across = (float)s;
                    const X87Reg turned = yaw - r;
                    yaw = (float)turned;
                    const double sine = X87_Sin(turned.v);
                    global<u32>(0x007909e0) = global<u32>(0x00790a38);
                    const double cosine = X87_Cos((double)yaw);
                    const float fc = (float)cosine, fs = (float)sine;
                    s1c = Bits(fc);
                    s2c = Bits(fc);
                    p = (Mul(s.v, cosine)) + Mul(sine, (double)s28);
                    q = X87Reg(fc) * s28 - X87Reg(fs) * across;
                }
            } else {
                EaseAngle(X87Reg(yaw), dt, paused);
                const double sine = X87_Sin((double)angle), cosine = X87_Cos((double)angle);
                const float fc = (float)cosine, fs = (float)sine;
                const float across = (float)s;
                s1c = Bits(across);
                s2c = Bits(fc);
                p = Mul(sine, (double)s28) + Mul(cosine, s.v);
                q = X87Reg(fc) * s28 - X87Reg(fs) * across;
            }
            const X87Reg r = X87_Sqrt(X87Reg(zoom)) * 0.0199999996f;
            const float oldX = vx, oldZ = vz;
            vy = 0.0f;
            const X87Reg nx = (r * p + vx) * 0.5f, nz = (r * q + vz) * 0.5f;
            vx = (float)nx;
            vz = (float)nz;
            const X87Reg back = kOne - X87Reg(dt) * kDecay;
            if ((edges & 1) && Below0(vx)) vx = (float)(oldX / back);
            if ((edges & 4) && Below0(vz)) vz = (float)(oldZ / back);
            if ((edges & 2) && vx > 0.0f) vx = (float)(oldX / back);
            if ((edges & 8) && vz > 0.0f) vz = (float)(oldZ / back);
        }
        if (!(std::fabs(vx) >= kSmall) && !(std::fabs(vy) >= kSmall) && !(std::fabs(vz) >= kSmall)) {
            global<u32>(0x00790a88) = 0;
            global<u32>(0x00790a8c) = 0;
            global<u32>(0x00790a90) = 0;
        }
        if ((flags & 0x16) && !(flags & 0x3c)) {
            if (WalkPoint(dt, s1c, s2c, s40)) {
                flags = flagsNow;
                k = keys;
            }
        } else {
            const X87Reg nx = X87Reg(dt) * vx + px, nz = X87Reg(dt) * vz + pz;
            px = (float)nx;
            pz = (float)nz;
        }
        if (k & 0x80) {
            if (k & 0x200) {
                if (!(zoom >= 40.0f)) zoom = 40.0f;
                else if (zoom > 300.0f) zoom = 300.0f;
            } else {
                if (!(zoom >= 70.0f)) zoom = 70.0f;
                else if (zoom > 130.0f) zoom = 130.0f;
            }
        }
        global<u32>(0x00790ab8) = global<u32>(0x007908f0);
        global<u32>(0x00790abc) = global<u32>(0x007908f4);
        global<u32>(0x00790ac0) = global<u32>(0x007908f8);
        float &eyeX = global<float>(0x00790aa0);
        float &eyeY = global<float>(0x00790aa4);
        float &eyeZ = global<float>(0x00790aa8);
        float &tilt = global<float>(0x00790a3c);
        bool ground = true;
        if (flags & 0x16) {
            eyeX = 0.0f;
            eyeZ = 0.0f;
            const u8 *const view = global<const u8 *>(0x007a0854);
            const u8 *const e = view ? at<const u8 *>(view, 0x6c) : nullptr;
            if (e && at<u32>(e, 0x28) && at<u32>(e, 0x2c) && !(flags & 0x3c)) {
                s1c = global<u32>(0x007908f0);
                s40 = global<u32>(0x007908f8);
            }
            ground = !(flags & 0x3c);
            if (ground) {
                eyeY = (float)(X87Reg(gEnv->height(s1c, s40, 1)) - -5.0f);
                flags = flagsNow;
            }
        } else {
            const X87Reg z8 = (X87Reg(zoom) - 70.0f) * 0.0166666675f;
            const X87Reg a = (kOne - z8) * kQuarterPi;
            const float back = (float)X87Reg(-(double)zoom);
            eyeZ = back;
            const X87Reg b = z8 * -1.13446403f;
            const X87Reg t = a - b;
            eyeX = 0.0f;
            tilt = (float)t;
            if (!(t.v > 0.0)) tilt = 0.0f;
            if (tilt >= 1.47079635f) tilt = 1.47079635f;
            const double sine = X87_Sin((double)tilt), cosine = X87_Cos((double)tilt);
            const X87Reg ey = Mul(sine, (double)back), ez = Mul(cosine, (double)back);
            eyeY = (float)X87Reg(-ey.v);
            eyeZ = (float)ez;
            const X87Reg h = (X87Reg(gEnv->height(global<u32>(0x007908f0), global<u32>(0x007908f8), 1)) + global<float>(0x00790ae4)) * 0.5f;
            global<float>(0x00790ae4) = (float)h;
            eyeY = (float)(h + eyeY);
            const auto lean = [](float v, float rate, u32 slot) {
                float &out = global<float>(slot);
                if (Below0(v)) {
                    const X87Reg q = X87_Sqrt(X87_MulFloatDouble(-v, kHundred));
                    out = (float)X87Reg(-(q * rate - 0.00100000005f).v);
                    if (!(0.0f >= out)) out = 0.0f;
                } else {
                    const X87Reg q = X87_Sqrt(X87_MulFloatDouble(v, kHundred));
                    out = (float)(q * rate - 0.00100000005f);
                    if (0.0f > out) out = 0.0f;
                }
            };
            lean(vx, kBlendRate, 0x007908e0);
            lean(vz, -kBlendRate, 0x007908e4);
            flags = flagsNow;
        }
        {
            const double sine = X87_Sin((double)angle), cosine = X87_Cos((double)angle);
            const float fc = (float)cosine, fs = (float)sine;
            const float oldEyeX = (float)X87Reg(eyeX);
            const X87Reg ex = Mul(sine, (double)eyeZ) + X87Reg(eyeX) * fc;
            const X87Reg h = X87Reg(global<float>(0x00790abc)) + eyeY;
            const X87Reg ec = Mul(cosine, (double)eyeZ);
            eyeX = (float)ex;
            const X87Reg ax = X87Reg(global<float>(0x00790ab8)) + eyeX;
            eyeY = (float)h;
            eyeX = (float)ax;
            eyeZ = (float)((ec - X87Reg(fs) * oldEyeX) + global<float>(0x00790ac0));
        }
        if (!(flags & 0x16)) {
            Matrix_FromPitchYaw(matrix, tilt, angle);
            Matrix_RotateEuler(matrix, 9, reinterpret_cast<const float *>((uintptr_t)0x007908e0));
        } else if (!(flags & 0x3c)) {
            Matrix_FromPitchYaw(matrix, global<float>(0x00790adc), angle);
        } else {
            const float *const part = global<const float *>(0x00790a30);
            std::memcpy(matrix, part, 0x40);
            float t = dt;
            if (dt > 1000.0f) t = 1000.0f;
            if (!(t >= kOne)) t = kOne;
            const float blend = (float)((X87Reg(1000.0f) - X87Reg(t)) * 0.000500000024f);
            const X87Reg rest = X87Reg(kOne) - blend;
            float *const old = reinterpret_cast<float *>((uintptr_t)0x00790948);
            for (int i = 0; i < 11; ++i)
                if ((i & 3) != 3) matrix[i] = (float)(X87Reg(blend) * old[i] + X87Reg(part[i]) * rest);
            std::memcpy(old, part, 0x40);
            float local[16];
            Matrix_Identity(local);
            const float angles[3] = {global<float>(0x00790adc), global<float>(0x00790ae0), 0.0f};
            Matrix_RotateEulerFast(local, 4, angles);
            Matrix_PreMul(local, matrix);
        }
        if (global<u8>(0x00790ab0) & 0x3c) {
            const u32 *const part = global<const u32 *>(0x00790a30);
            global<u32>(0x007909c0) = X87_FloatThrough(part[12]);
            global<u32>(0x007909c4) = X87_FloatThrough(part[13]);
            global<u32>(0x007909c8) = X87_FloatThrough(part[14]);
        } else {
            global<u32>(0x007909c0) = global<u32>(0x00790aa0);
            global<u32>(0x007909c4) = X87_FloatThrough(global<u32>(0x00790aa4));
            global<u32>(0x007909c8) = X87_FloatThrough(global<u32>(0x00790aa8));
        }
        float *const last = reinterpret_cast<float *>((uintptr_t)0x00790a48);
        if (!gEnv->matricesClose(matrix, last)) {
            Mat4_Assign(global<u32 *>(0x0087afe8), nullptr, reinterpret_cast<const u32 *>(matrix));
        } else {
            yaw = (float)std::fmod((double)yaw, kTwoPiD);   // _CIfmod (0x67b24a), exact
            angle = (float)std::fmod((double)angle, kTwoPiD);
        }
        std::memcpy(last, matrix, 0x40);
        global<float *>(0x00790a98) = matrix;
        global<u32>(0x007909e4) = now;
    }
    float inverse[16];
    const float *const from = Matrix_Invert(inverse, global<const float *>(0x00790a98));
    std::memcpy(reinterpret_cast<void *>((uintptr_t)0x00790900), from, 0x40);
}
}
