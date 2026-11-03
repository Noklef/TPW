#include "handles.h"

#include <cmath>
#include <cstdint>

#include "../math/x87.h"
#include "helpers.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
u8 *Ptr(const void *p, u32 offset) { return at<u8 *>(p, offset); }

const float kHalf = 0.5f;           // 0x6fdeac
const float kMinusThird = -0.333333343f;  // 0x6fdebc
const float kZero = 0.0f;           // 0x6fde74
const float kSharpFolded = -0.999f; // 0x6fdeb4
const float kSharpStraight = 0.999f;// 0x6fdeb8

struct V3 {
    float x, y, z;
};

V3 Toward(const u8 *to, const u8 *from) {
    return {(float)(X87Reg(at<float>(to, 0x4c)) - at<float>(from, 0x4c)), (float)(X87Reg(at<float>(to, 0x50)) - at<float>(from, 0x50)),
            (float)(X87Reg(at<float>(to, 0x54)) - at<float>(from, 0x54))};
}

V3 Over(const V3 &v, float by) { return {(float)(X87Reg(v.x) / by), (float)(X87Reg(v.y) / by), (float)(X87Reg(v.z) / by)}; }
V3 Negative(const V3 &v) { return {-v.x, -v.y, -v.z}; }

const u8 *Info(const u8 *piece) { return Ptr(Ptr(piece, 0x100), 0x18); }
bool Railed(const u8 *info) { return (at<u32>(info, 0) & 0x4000) != 0; }

// Not railed, with a railed info on either neighbour's link: the handles stay flat.
bool Flatten(const u8 *piece) {
    const u8 *info = Info(piece);
    if (!info || Railed(info)) return false;
    const u8 *other = Ptr(piece, 0x68);
    if (other) {
        info = Info(other);
        if (info && Railed(info)) return true;
    }
    other = Ptr(piece, 0x64);
    if (!other) return false;
    info = Info(other);
    return info && Railed(info);
}

// The game's acos (0x67b4e0), as in motion/quat.cpp.
double CrtAcos(float x) {
    const float r = std::sqrt((1.0f + x) * (1.0f - x));
    return std::atan2((double)r, (double)x);
}

float Half(float from, float less, float by, float length) { return (float)((X87Reg(from) - less) * kHalf / by * length); }

void Finish(u8 *piece, const V3 &a, const V3 &b) {
    at<V3>(piece, 0x28) = a;
    at<V3>(piece, 0x34) = b;
    const float px = at<float>(piece, 0x4c), py = at<float>(piece, 0x50), pz = at<float>(piece, 0x54);
    at<float>(piece, 0x40) = (float)(px - X87Reg(a.x) * kMinusThird);
    at<float>(piece, 0x44) = (float)(py - X87Reg(a.y) * kMinusThird);
    at<float>(piece, 0x48) = (float)(pz - X87Reg(a.z) * kMinusThird);
    at<float>(piece, 0x58) = (float)(px - X87Reg(b.x) * kMinusThird);
    at<float>(piece, 0x5c) = (float)(py - X87Reg(b.y) * kMinusThird);
    at<float>(piece, 0x60) = (float)(pz - X87Reg(b.z) * kMinusThird);
}

}  // namespace

void __cdecl TrackPiece_Handles(u8 *piece, const u8 *rules, const u8 *coaster) {
    const u8 *info = Info(piece);
    u32 flags = at<u32>(piece, 8);
    const u8 *prev = flags & 0x80000 ? nullptr : Ptr(piece, 0x68);
    const u8 *next = flags & 0x40000 ? nullptr : Ptr(piece, 0x64);
    if (flags & 0x100000) next = Ptr(Ptr(coaster, 0x18), 0x64);
    flags &= 0xf1ffffff;
    at<u32>(piece, 8) = flags;
    if (!prev && !next) return;

    const float tension = at<float>(rules, 0xc);
    V3 a, b;

    if (!next) {
        const V3 way = Toward(prev, piece);
        bool flatten = false;
        if (info && Railed(info) && !(flags & 0x80)) {
            const u8 *before = Ptr(prev, 0x68);
            bool wayIsB = false;
            if (before) {
                const float x = at<float>(prev, 0x4c), y = at<float>(prev, 0x50), z = at<float>(prev, 0x54);
                const X87Reg bend = (X87Reg(y) - at<float>(before, 0x50)) * (X87Reg(y) - at<float>(piece, 0x50)) +
                                    (X87Reg(z) - at<float>(before, 0x54)) * (X87Reg(z) - at<float>(piece, 0x54)) +
                                    (X87Reg(x) - at<float>(before, 0x4c)) * (X87Reg(x) - at<float>(piece, 0x4c));
                wayIsB = !(bend.v >= at<float>(info, 0xb0));
            }
            if (wayIsB) {
                b = Over(way, tension);
                a = Negative(b);
            } else {
                a = Over(way, tension);
                b = Negative(a);
            }
        } else {
            a = Over(way, tension);
            b = Negative(a);
            flatten = Flatten(piece);
        }
        at<u32>(piece, 0x94) = 0;
        if (flatten) a.y = b.y = 0.0f;
        Finish(piece, a, b);
        return;
    }

    const V3 toNext = Toward(next, piece);
    if (!prev) {
        b = Over(toNext, tension);
        a = Negative(b);
        at<u32>(piece, 0x94) = 0;
        Finish(piece, a, b);
        return;
    }
    const V3 toPrev = Toward(prev, piece);

    const float l1 = (float)X87_Sqrt(X87Reg(toPrev.x) * X87Reg(toPrev.x) + X87Reg(toPrev.y) * toPrev.y + X87Reg(toPrev.z) * toPrev.z);
    const float l2 = (float)X87_Sqrt(X87Reg(toNext.x) * X87Reg(toNext.x) + X87Reg(toNext.y) * toNext.y + X87Reg(toNext.z) * toNext.z);
    const X87Reg r1x = X87Reg(toPrev.x) / l1, r1y = X87Reg(toPrev.y) / l1, r1z = X87Reg(toPrev.z) / l1;
    const V3 n1 = {(float)r1x, (float)r1y, (float)r1z};
    const V3 n2 = Over(toNext, l2);
    const float dot = (float)(r1x * n2.x + r1y * n2.y + r1z * n2.z);

    V3 u = n1, w = n2;
    Vec3_NormalizeXZ(&u.x);
    Vec3_NormalizeXZ(&w.x);
    float c = (float)(X87Reg(u.x) * w.x + X87Reg(u.z) * w.z);
    if (-1.0f > c) c = -1.0f;
    if (!(1.0f >= c)) c = 1.0f;
    const float cross = (float)(X87Reg(u.z) * w.x - X87Reg(u.x) * w.z);   // (with CRT _isnan: a NaN is not below 0)
    const double turn = CrtAcos(c);
    at<float>(piece, 0x94) = (float)(cross < kZero ? -turn : turn);

    const bool low = !(dot >= (info ? at<float>(info, 0xb0) : kSharpFolded));
    const bool railed = info && Railed(info);
    if (low || dot > kSharpStraight) {
        if (railed) {
            const V3 &p = flags & 0x80 ? n1 : n2, &q = flags & 0x80 ? n2 : n1;
            a = {Half(p.x, q.x, tension, l1), Half(p.y, q.y, tension, l1), Half(p.z, q.z, tension, l1)};
            b = {Half(q.x, p.x, tension, l2), Half(q.y, p.y, tension, l2), Half(q.z, p.z, tension, l2)};
            Finish(piece, a, b);
            return;
        }
        a = Over(toPrev, tension);
        b = Over(toNext, tension);
    } else {
        const float gain = at<float>(rules, 8);
        V3 h = {(float)(X87Reg(n1.x) + n2.x), (float)(X87Reg(n1.y) + n2.y), (float)(X87Reg(n1.z) + n2.z)};
        if (railed) {
            const X87Reg g1 = X87Reg(l1) * gain;
            const float g2 = (float)(X87Reg(l2) * gain);
            a = {(float)(X87Reg(h.x) * g1), (float)(X87Reg(h.y) * g1), (float)(X87Reg(h.z) * g1)};
            b = {(float)(X87Reg(-h.x) * g2), (float)(X87Reg(-h.y) * g2), (float)(X87Reg(-h.z) * g2)};
            Finish(piece, a, b);
            return;
        }

        V3 k = {(float)(X87Reg(toNext.z) * toPrev.y - X87Reg(toNext.y) * toPrev.z), (float)(X87Reg(toNext.x) * toPrev.z - X87Reg(toNext.z) * toPrev.x),
                (float)(X87Reg(toNext.y) * toPrev.x - X87Reg(toNext.x) * toPrev.y)};
        if ((at<u8>(rules, 0) & 8) && !(flags & 0x40) && !(k.y >= kZero)) {
            at<u32>(piece, 8) = flags | 0x10000000;
        } else {
            at<u32>(piece, 8) &= 0xefffffff;
        }

        const float lh = (float)X87_Sqrt(X87Reg(h.x) * X87Reg(h.x) + X87Reg(h.y) * h.y + X87Reg(h.z) * h.z);
        const float lk = (float)X87_Sqrt(X87Reg(k.x) * X87Reg(k.x) + X87Reg(k.y) * k.y + X87Reg(k.z) * k.z);
        const X87Reg hx = X87Reg(h.x) / lh, hy = X87Reg(h.y) / lh, hz = X87Reg(h.z) / lh;
        const float fx = (float)hx, fy = (float)hy, fz = (float)hz;
        k = Over(k, lk);

        const X87Reg t1 = hy * k.z - hz * k.y;
        const float t2 = (float)(X87Reg(fz) * k.x - hx * k.z);
        const float t3 = (float)(X87Reg(fx) * k.y - X87Reg(fy) * k.x);
        const float g1 = (float)(X87Reg(l1) * gain);
        const X87Reg g2 = X87Reg(l2) * gain;
        const float bx = -(float)t1;
        a = {(float)(t1 * g1), (float)(X87Reg(t2) * g1), (float)(X87Reg(t3) * g1)};
        b = {(float)(X87Reg(bx) * g2), (float)(X87Reg(-t2) * g2), (float)(X87Reg(-t3) * g2)};
    }
    if (Flatten(piece)) a.y = b.y = 0.0f;
    Finish(piece, a, b);
}
