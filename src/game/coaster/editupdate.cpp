#include "editupdate.h"

#include <cstdint>
#include <cstring>
#include <utility>

#include "../core/globals.h"
#include "../math/matrix.h"
#include "../math/sintable.h"
#include "../math/x87.h"
#include "cursor.h"
#include "helpers.h"
#include "placecheck.h"
#include "registry.h"
#include "train.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &g(u32 address) { return *reinterpret_cast<T *>(address); }

const EditUpdateEnv kPortEnv = {&CoasterLink_FindCarOnRide, &TrackSelection_ClearFlags, &Flags_SetChanged,      &TrackSelection_SetFirst,
                                &CoasterLinks_FindAtFocus,  &Coaster_DropCar,           &Car_SetMoveCostChange, &CoasterPiece_CanPlaceAt,
                                &Car_Edited,                &Car_TravelCost};
const EditUpdateEnv *g_env = &kPortEnv;

const float kMinusHalfPi = -1.57079637f;  // 0x6fe1ec
const float kTenth = 0.100000001f;        // 0x6fe1f0

u32 Flags() { return g<u32>(0x0079c658); }
u8 *Link() { return g<u8 *>(0x00790fec); }
u8 *Car() { return g<u8 *>(0x0079c640); }

// fcom's "equal" (C3): equal or unordered.
bool same(float a, float b) { return !(a < b || a > b); }

// A dword moved (mov): a float's bits as they are.
void copy(float *to, u32 from) { std::memcpy(to, reinterpret_cast<const void *>((uintptr_t)from), 4); }
void store(u32 to, const float &from) { std::memcpy(reinterpret_cast<void *>((uintptr_t)to), &from, 4); }

// *p = v, then held to [lo, hi] (fcom: below lo or unordered takes lo, above hi takes hi). Whether it was held.
bool hold(float *p, X87Reg v, u32 lo, u32 hi) {
    *p = (float)v;
    if (!(v.v >= (double)g<float>(lo))) {
        copy(p, lo);
        return true;
    }
    if (v.v > (double)g<float>(hi)) {
        copy(p, hi);
        return true;
    }
    return false;
}

// The sine table's entry for an angle already scaled (fistp of the float).
X87Reg sine(float scaled) { return X87Reg(SinTable_At(SinTable_Game(), scaled)); }
const float &scale() { return g<float>(0x007b095c); }
// Its scaled angles: scale * a, and (a - -pi/2) * scale (cosines) where the original does not store a - -pi/2 first.
float scaled(float a) { return (float)(X87Reg(scale()) * a); }
float scaled_cos(float a) { return (float)((X87Reg(a) - kMinusHalfPi) * scale()); }

// About y: x = s z + x c, z = c z - x s.
void turn_xz(float *v, X87Reg s, X87Reg c) {
    const X87Reg x = v[0];
    const X87Reg sz = s * v[2];
    v[0] = (float)(sz + x * c);
    const X87Reg cz = c * v[2];
    v[2] = (float)(cz - x * s);
}

// About x: y = y c - s z, z = c z + y s.
void turn_yz(float *v, X87Reg s, X87Reg c) {
    const X87Reg y = v[1];
    const X87Reg yc = y * c;
    v[1] = (float)(yc - s * v[2]);
    const X87Reg cz = c * v[2];
    v[2] = (float)(cz + y * s);
}

// The vector v set from the three floats at `from` and turned about y by -heading, about x by dy and about y by heading (the cosines'
// angles less -pi/2 given: tdy, theading).
void turn_vector(float *v, u32 from, float neg, float dy, float tdy, float heading, float theading) {
    copy(v, from);
    copy(v + 1, from + 4);
    copy(v + 2, from + 8);
    turn_xz(v, sine(scaled(neg)), sine(scaled_cos(neg)));
    turn_yz(v, sine(scaled(dy)), sine(scaled(tdy)));
    turn_xz(v, sine(scaled(heading)), sine(scaled(theading)));
}

// The span tool's ends for the hovered ride (0x7a1cd4).
void select_span(u32 ride) {
    u8 *const car = ride ? g_env->findCarOnRide(ride) : nullptr;
    u8 *first = g<u8 *>(0x0079c6dc), *second;
    if (!car) {
        second = nullptr;
    } else if (car == first) {
        if (at<u8>(at<u8 *>(first, 0x100), 4) & 2) {
            g_env->clearSelectionFlags();
            g_env->setChanged(g<u32 *>(0x00790fec), 0);
            return;
        }
        const u32 flags = at<u32>(car, 8);
        u8 *const next = at<u8 *>(car, 0x68), *const prev = at<u8 *>(car, 0x64);
        first = next;
        second = prev;
        if (flags & 0x40 && flags & 0x80) {
            second = at<u8 *>(prev, 0x64);
            if (!first) first = car;
            if (!second) second = prev;
        } else if (flags & 0x40) {
            first = at<u8 *>(next, 0x68);
            if (!first) first = next;
            if (!second) second = car;
        } else {
            if (!first) first = car;
            if (!second) second = car;
        }
    } else {
        second = car;
        if (!at<u32>(car, 0))
            second = at<u8 *>(Link(), 0x58);
        else if (!at<u32>(first, 0))
            first = at<u8 *>(Link(), 0x58);
        if (at<u32>(first, 0) > at<u32>(second, 0)) std::swap(first, second);
        u32 flags = at<u32>(first, 8);
        if (flags & 0x40 && flags & 0x80 && at<u8 *>(first, 0x68)) first = at<u8 *>(first, 0x68);
        flags = at<u32>(second, 8);
        if (flags & 0x40 && !(flags & 0x80) && at<u8 *>(second, 0x64)) second = at<u8 *>(second, 0x64);
    }
    g_env->clearSelectionFlags();
    g<u8 *>(0x0079c62c) = first;
    g_env->setFirst(second, 1);
}

// The move tool: the piece 0x79c6a4 dropped on the focus tile (x, z) of the link.
void move_piece(u8 *link, s32 x, s32 z) {
    u8 *piece = g<u8 *>(0x0079c6a4);
    if (at<s32>(at<u8 *>(piece, 0x100), 0xc) != x || at<s32>(at<u8 *>(piece, 0x100), 0x10) != z) {
        u8 *const found = g_env->findAtFocus(nullptr, nullptr, nullptr);
        g_env->setChanged(g<u32 *>(0x00790fec), 0);
        piece = g<u8 *>(0x0079c6a4);
        bool drop = true;
        if (found) {
            const u8 *const def = at<u8 *>(piece, 0x100);
            drop = at<u32>(found, 0x18) == at<u32>(def, 0x18) && at<u32>(found, 0x14) == at<u32>(def, 0x14) &&
                   at<u32>(found, 0x30) < at<u32>(at<u8 *>(found, 0x18), 0xcc);
        }
        if (drop) {
            float outX, outZ;
            g_env->dropCar(link, piece, found, x, z, &outX, &outZ, 1, 1);
            g_env->moveCostChange(g<u8 *>(0x0079c6a4), &g<float>(0x0079c698), &g<float>(0x0079c64c), &outX, &outZ, x, z);
            const s32 fits = g_env->canPlaceAt(x, z, nullptr, 0, 0, 0, 0);
            if (u8 *const p = g<u8 *>(0x0079c6a4)) {
                if (fits)
                    at<u32>(p, 8) &= ~0x4000u;
                else
                    at<u32>(p, 8) |= 0x4000;
            }
            if (u8 *const next = at<u8 *>(g<u8 *>(0x0079c6a4), 0x68)) {
                if (fits)
                    at<u32>(next, 8) &= ~0x4000u;
                else
                    at<u32>(next, 8) |= 0x4000;
            }
        }
    }
    if (!g<u32>(0x00785a2c)) {
        s32 cost = g<s32>(0x0079c69c);
        if (cost < 0) cost += (s32)0xf1111112;
        Global81af4c_Add((u32)cost);
    }
}

}  // namespace

void EditUpdate_SetEnv(const EditUpdateEnv *env) { g_env = env ? env : &kPortEnv; }

extern "C" {

void __cdecl CoasterEdit_Update() {
    if (!Link() || !at<u32>(Link(), 0x40)) return;
    u32 edited = 0;
    float h;
    if (u8 *const car = Car())
        h = (float)(X87Reg(g<float>(0x00790a38)) - at<float>(car, 0xe0));
    else
        copy(&h, 0x00790a38);
    u32 flags = Flags();
    if (flags & 0x3f && flags & 0x1e00) {
        float dx = (float)((X87Reg(g<float>(0x0079c630)) - g<float>(0x00790ac8)) * g<float>(0x0079c6ec));
        float dy = (float)((X87Reg(g<float>(0x0079c634)) - g<float>(0x00790acc)) * g<float>(0x0079c6a8));
        if (flags & 0x2000) {   // (turned by the camera)
            const X87Reg s = sine(scaled(h));
            const float c = (float)sine(scaled_cos(h));
            const X87Reg y = s * dx + X87Reg(c) * dy;
            const X87Reg x = X87Reg(c) * dx - s * dy;
            dx = (float)x;
            dy = (float)y;
        }
        flags = Flags();
        bool run = false;   // (the y value or vector)
        if (flags & 0x1400) run = (flags & 0xa00 && !same(dx, g<float>(0x0079c668))) || !same(dy, g<float>(0x0079c618));
        if (run) {
            if (flags & 0x400) {
                float *const p = g<float *>(0x0079c6fc);
                if (hold(p, X87Reg(dy) + g<float>(0x0079c6d4), 0x0079c6e4, 0x0079c66c) && !(Flags() & 0x2000))
                    g<float>(0x0079c634) = (float)((X87Reg(*p) - g<float>(0x0079c6d4)) / g<float>(0x0079c6a8) + g<float>(0x00790acc));
                if (float *const q = g<float *>(0x0079c6f8)) hold(q, X87Reg(dy) + g<float>(0x0079c6d0), 0x0079c6e8, 0x0079c670);
            } else if (flags & 0x1000) {
                float *const p = g<float *>(0x0079c6fc);
                copy(p, 0x0079c6b0);
                copy(p + 1, 0x0079c6b4);
                copy(p + 2, 0x0079c6b8);
                const double a = Vec3_AngleY(p);
                const float heading = (float)a, neg = (float)-a;
                turn_xz(p, sine(scaled(neg)), sine(scaled_cos(neg)));
                const float tdy = (float)(X87Reg(dy) - kMinusHalfPi);
                turn_yz(p, sine(scaled(dy)), sine(scaled(tdy)));
                const float theading = (float)(X87Reg(heading) - kMinusHalfPi);
                turn_xz(p, sine(scaled(heading)), sine(scaled(theading)));
                if (float *const q = g<float *>(0x0079c6f8)) turn_vector(q, 0x0079c6c0, neg, dy, tdy, heading, theading);
            }
            store(0x0079c618, dy);
            edited = 1;
            flags = Flags();
        }
        bool runX = false;   // (the x value or vector)
        if (flags & 0xa00) runX = !same(dx, g<float>(0x0079c668)) || (flags & 0x1400 && !same(dy, g<float>(0x0079c618)));
        if (runX) {
            if (flags & 0x200) {
                float *const p = g<float *>(0x0079c614);
                if (hold(p, X87Reg(dx) + g<float>(0x0079c6f0), 0x0079c61c, 0x0079c65c) && !(Flags() & 0x2000))
                    g<float>(0x0079c630) = (float)((X87Reg(*p) - g<float>(0x0079c6f0)) / g<float>(0x0079c6ec) + g<float>(0x00790ac8));
                if (float *const q = g<float *>(0x0079c610)) hold(q, X87Reg(dx) + g<float>(0x0079c6f4), 0x0079c620, 0x0079c654);
            } else if (flags & 0x800) {
                float *const p = g<float *>(0x0079c614);
                copy(p, 0x0079c688);
                copy(p + 1, 0x0079c68c);
                copy(p + 2, 0x0079c690);
                const float tdx = (float)(X87Reg(dx) - kMinusHalfPi);
                turn_xz(p, sine(scaled(dx)), sine(scaled(tdx)));
                if (float *const q = g<float *>(0x0079c610)) {
                    copy(q, 0x0079c678);
                    copy(q + 1, 0x0079c67c);
                    copy(q + 2, 0x0079c680);
                    turn_xz(q, sine(scaled(dx)), sine(scaled(tdx)));
                }
            }
            store(0x0079c668, dx);
            edited = 1;
            flags = Flags();
        }
    }
    if (flags & 0x13f) {
        const s32 x = (s32)(long long)(X87Reg(g<float>(0x007a1c70)) * kTenth).v;  // (__ftol)
        const s32 z = (s32)(long long)(X87Reg(g<float>(0x007a1c78)) * kTenth).v;
        flags = Flags();
        g<s32>(0x0079c650) = x;
        g<s32>(0x0079c648) = z;
        if (flags & 0x180) {
            const u32 ride = g<u32>(0x007a1cd4);
            u8 *const link = Link();
            if (flags & 0x80 && ride != g<u32>(0x0079c6ac))
                select_span(ride);
            else if (flags & 0x100)
                move_piece(link, x, z);
        }
        g<u32>(0x0079c6ac) = g<u32>(0x007a1cd4);
    }
    if (edited && Car()) g_env->carEdited(Flags(), Car(), Link(), 0, 0);
    if (g<u8>(0x0079c658) & 0x3f) {
        u8 *const car = Car();
        if (car && !(at<u8>(at<u8 *>(car, 0x100), 4) & 2) && !g<u32>(0x00785a2c)) {
            const s32 old = (s32)(long long)(double)g<float>(0x0079c698);  // (__ftol)
            const double cost = g_env->travelCost(Link(), 0, Car(), nullptr, nullptr, nullptr, nullptr);
            s32 change = Float_RoundUp((float)cost) - old;
            if (change < 0) change += (s32)0xf1111112;
            Global81af4c_Add((u32)change);
        }
    }
    if (g<u8>(0x0079c624) & 1) g_env->setChanged(g<u32 *>(0x00790fec), 0);
}

}  // extern "C"
