#include "handles.h"

#include <cmath>
#include <cstdint>
#include <cstring>
#include <emmintrin.h>

#include "../math/x87.h"
#include "cursor.h"
#include "helpers.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
u8 *Ptr(const void *p, u32 offset) { return at<u8 *>(p, offset); }

const float kHalf = 0.5f;           // 0x6fdeac
const float kMinusThird = -0.333333343f;  // 0x6fdebc
const float kZero = 0.0f;           // 0x6fde74
const float kSharpFolded = -0.999f; // 0x6fdeb4
const float kSharpStraight = 0.999f;// 0x6fdeb8
const float kTwoPi = 6.28318548f;   // 0x6fdec0
const float kMinusHalfPi = -1.57079637f;  // 0x6fdeb0
const float kMinusOne = -1.0f;      // 0x6fde9c
const float kEndHandle = 1.70000005f;  // 0x6fded4
const float kRailFar = -2.0999999f; // 0x6fdec4

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

// fld, fchs, fstp: a signalling NaN quieted and the sign flipped.
float Negated(float x) {
    u32 bits;
    std::memcpy(&bits, &x, 4);
    bits = X87_FloatThrough(bits) ^ 0x80000000u;
    std::memcpy(&x, &bits, 4);
    return x;
}

X87Reg Length(float x, float y, float z) {
    const X87Reg rx(x), ry(y), rz(z);
    return X87_Sqrt(rx * rx + ry * ry + rz * rz);
}

// (x, y, z) by the length, itself stored as a float first.
void Normalized(float x, float y, float z, float *out) {
    const float length = (float)Length(x, y, z);
    out[0] = (float)(X87Reg(x) / length);
    out[1] = (float)(X87Reg(y) / length);
    out[2] = (float)(X87Reg(z) / length);
}

// The sine table's entry for an angle by the table's scale, stored as a float first (fistp, rounded to nearest).
float SineAt(X87Reg angle) {
    const float index = (float)(angle * *reinterpret_cast<const float *>(0x007b095c));
    return reinterpret_cast<const float *>(0x007b0968)[_mm_cvtss_si32(_mm_set_ss(index)) & *reinterpret_cast<const u32 *>(0x007b0960)];
}

// The two comparisons with 0 and 1: a NaN comes out 0.
X87Reg Clamped01(X87Reg x) {
    if (!(x.v > 0.0)) x = X87Reg(0.0f);
    if (!(x.v < 1.0)) x = X87Reg(1.0f);
    return x;
}

// (1 - t) * from - t * -2.1 (0x6fdec4).
float TowardRail(X87Reg t, float from) {
    return (float)((X87Reg(1.0f) - t) * from - t * kRailFar);
}

// The size of a railed piece's handles from rules +8, by the piece's way to its neighbour and its angles at +0xec and +0xf8.
float RailedSize(const u8 *rules, const u8 *owner) {
    const float size = at<float>(rules, 8);
    const u8 *const other = Ptr(owner, at<u8>(owner, 8) & 0x80 ? 0x64 : 0x68);
    X87Reg dx(kZero);
    float dz = 1.0f;
    if (other) {
        dx = X87Reg(at<float>(other, 0x4c)) - at<float>(owner, 0x4c);
        dz = (float)(X87Reg(at<float>(other, 0x54)) - at<float>(owner, 0x54));
    }
    const float length = (float)X87_Sqrt(dx * dx + X87Reg(dz) * dz);
    float nx = (float)(dx / length), nz = (float)(X87Reg(dz) / length);
    if (nx != nx) {
        nx = 0.0f;
        nz = 1.0f;
    }
    const X87Reg angle = X87Reg(at<float>(owner, 0xec)) * kTwoPi;
    const float sine = SineAt(angle);
    const float angleF = (float)angle;
    const float cosine = SineAt(X87Reg(angleF) - kMinusHalfPi);
    const X87Reg facing = X87Reg(sine) * kMinusOne * nx + X87Reg(cosine) * kMinusOne * nz;
    const X87Reg dot = Clamped01(X87Reg(std::fabs(facing.v)));
    const float bySlope = TowardRail(dot * dot, size);
    const X87Reg left(std::fmod((double)at<float>(owner, 0xf8), 0.5));
    const X87Reg phase = Clamped01(left + left);
    const X87Reg swing = (X87Reg(SineAt(phase * kTwoPi - kMinusHalfPi)) - kMinusOne) * kHalf;
    return TowardRail(swing, bySlope);
}

}  // namespace

void __cdecl TrackNode_Handles(const u8 *rules, const float *way, u8 *node, float before, float after, const u8 *info, void *, void *, u32, u32) {
    std::memcpy(node + 0x28, way, 12);
    at<float>(node, 0x1c) = Negated(at<float>(node, 0x28));
    at<float>(node, 0x20) = Negated(at<float>(node, 0x2c));
    at<float>(node, 0x24) = Negated(at<float>(node, 0x30));
    X87Reg sizeIn(0.0f), sizeOut(0.0f);
    if (info && at<u32>(info, 0) & 0x4000) {
        const u8 *const owner = Ptr(node, 0x10);
        const float size = RailedSize(rules, owner);
        const auto railed = [](const u8 *other) { return other && at<u8>(other, 8) & 0x40; };
        if (at<u8>(owner, 8) & 0x80) {
            sizeIn = railed(Ptr(owner, 0x68)) ? X87Reg(size) * before : X87Reg(before) * kEndHandle;
            sizeOut = X87Reg(size) * after;
        } else {
            sizeIn = X87Reg(size) * before;
            sizeOut = railed(Ptr(owner, 0x64)) ? X87Reg(size) * after : X87Reg(after) * kEndHandle;
        }
    } else {
        sizeIn = X87Reg(before) * at<float>(rules, 8);
        sizeOut = X87Reg(after) * at<float>(rules, 8);
    }
    const X87Reg in = sizeIn / Length(at<float>(node, 0x1c), at<float>(node, 0x20), at<float>(node, 0x24));
    at<float>(node, 0x1c) = (float)(X87Reg(at<float>(node, 0x1c)) * in);
    at<float>(node, 0x20) = (float)(in * at<float>(node, 0x20));
    at<float>(node, 0x24) = (float)(in * at<float>(node, 0x24));
    const X87Reg out = sizeOut / Length(at<float>(node, 0x28), at<float>(node, 0x2c), at<float>(node, 0x30));
    at<float>(node, 0x28) = (float)(X87Reg(at<float>(node, 0x28)) * out);
    at<float>(node, 0x2c) = (float)(out * at<float>(node, 0x2c));
    at<float>(node, 0x30) = (float)(out * at<float>(node, 0x30));
    for (u32 k = 0; k < 3; ++k) {
        const float place = at<float>(node, 0x40 + 4 * k);
        at<float>(node, 0x34 + 4 * k) = (float)(place - X87Reg(at<float>(node, 0x1c + 4 * k)) * kMinusThird);
        at<float>(node, 0x4c + 4 * k) = (float)(place - X87Reg(at<float>(node, 0x28 + 4 * k)) * kMinusThird);
    }
}

void __cdecl TrackPiece_NodeHandles(const u8 *piece, const u8 *source) {
    const u8 *const rules = Ptr(*reinterpret_cast<u8 *const *>(0x00792218), 4);
    const u8 *const info = Ptr(Ptr(piece, 0x100), 0x18);
    u8 *first = Ptr(Ptr(piece, 0x90), 4), *second = Ptr(Ptr(piece, 0x90), 8);
    for (u32 i = 0; i < at<u32>(Ptr(piece, 0x90), 0); ++i, first += 0x58, second += 0x58) {
        float way[3];
        if (source) {
            const u8 *const list = Ptr(Ptr(Ptr(source, 8), 0x28), 4);
            const u8 *const v = Ptr(list, at<u32>(first, 4) * 20 + 4);
            Normalized(at<float>(v, 0x20), at<float>(v, 0x24), at<float>(v, 0x28), way);
        } else if (at<u32>(piece, 8) & 0x40) {
            way[0] = 0.0f;
            way[1] = at<u32>(piece, 8) & 0x80 ? 1.0f : -1.0f;
            way[2] = 0.0f;
        } else {
            const u8 *const v = Ptr(Ptr(piece, 0x90), 4);  // (the list's first node each time, sic: its handle as this loop left it)
            Normalized(at<float>(v, 0x28), at<float>(v, 0x2c), at<float>(v, 0x30), way);
        }
        float before, after, before2, after2, beforeWay[3], afterWay[3];
        TrackPiece_NodeGaps(i, piece, &before, &after, 1, beforeWay, afterWay);
        TrackPiece_NodeGaps(i, piece, &before2, &after2, 0, nullptr, nullptr);
        const u32 isFirst = i == 0, isLast = i == at<u32>(Ptr(piece, 0x90), 0) - 1;
        TrackNode_Handles(rules, way, first, before, after, info, beforeWay, afterWay, isFirst, isLast);
        TrackNode_Handles(rules, way, second, before2, after2, info, beforeWay, afterWay, isFirst, isLast);
    }
}

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
