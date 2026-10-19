#include "nodes.h"

#include <cmath>
#include <cstdint>
#include <cstring>

#include "../core/globalmem.h"
#include "../math/matrix.h"
#include "../math/sintable.h"
#include "../math/x87.h"
#include "cars.h"
#include "cursor.h"
#include "helpers.h"
#include "orient.h"
#include "spline.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) {
    return *reinterpret_cast<T *>((uintptr_t)p + offset);
}

// lo <= x <= hi, as fcom lo (below or unordered: out) then fcomp hi (above: out; a NaN hi passes).
bool within(float x, float lo, float hi) { return x >= lo && !(x > hi); }

const float kStep = 0.05f;               // 0x3d4ccccd
const float kMinusOne = -1.0f;           // 0x6fe140
const float kMinusThree = -3.0f;         // 0x6fe114
const float kMinusHalfPi = -1.57079637f; // 0x6fe138

// Node index of a piece's list (+0x90): +4 the spline nodes, +8 the second list.
u8 *ListNode(u32 piece, u32 index, u32 list = 4) {
    return at<u8 *>(at<u8 *>(reinterpret_cast<const void *>((uintptr_t)piece), 0x90), list) + index * 0x58;
}

// A cursor at the first piece's node 0: its end node 1 when it has more than one, else the next piece's node 0.
void Begin(u32 *cursor) {
    u8 *const first = at<u8 *>(*reinterpret_cast<u8 *const *>(0x00790fe4), 0x18);
    cursor[0] = 0;
    cursor[1] = (u32)(uintptr_t)first;
    if (at<u32>(at<u8 *>(first, 0x90), 0) > 1) {
        cursor[2] = 1;
        cursor[3] = (u32)(uintptr_t)first;
    } else {
        cursor[2] = 0;
        cursor[3] = at<u32>(first, 0x64);
    }
}

// The node after the car at coaster+0x54: node 0 of the piece after it.
u8 *WrapNode(const u8 *coaster) { return ListNode(at<u32>(at<u8 *>(coaster, 0x54), 0x64), 0); }

// x*x + y*y + z*z (x on the stack, y and z floats), its root.
X87Reg Length(X87Reg x, float y, float z) { return X87_Sqrt(x * x + X87Reg(y) * y + X87Reg(z) * z); }

u32 bits(float f) {
    u32 b;
    std::memcpy(&b, &f, 4);
    return b;
}

}  // namespace

extern "C" {

void __cdecl CoasterNode_Lerp(const u8 *coaster, u32 index, float t, float *out) {
    const u8 *nodes = at<const u8 *>(coaster, 0x9c);
    const float *a = &at<const float>(nodes, index * 0x38 + 4);
    const float *b = &at<const float>(nodes, (index + 1) * 0x38 + 4);
    const X87Reg u = X87Reg(1.0f) - t;
    out[0] = (float)(X87Reg(a[0]) * u + X87Reg(b[0]) * t);
    out[1] = (float)(X87Reg(b[1]) * t + X87Reg(a[1]) * u);
    out[2] = (float)(X87Reg(a[2]) * u + X87Reg(b[2]) * t);
}

u32 __cdecl Coaster_IsPastEnd(const u8 *coaster, float x) {
    return x >= at<float>(coaster, 0xa8) ? 1 : 0;
}

u32 __cdecl Coaster_FindZone(const u8 *owner, u32, const float *pos, float t) {
    const u32 count = at<u32>(owner, 0x7c);
    const u8 *boxes = at<const u8 *>(owner, 0x78);
    u32 found = 0;
    for (u32 i = 0; i < count - 1; ++i) {
        const float *box = &at<const float>(boxes, i * 0x2c);
        if (within(t, box[0], box[1]) && within(pos[1], box[2], box[3]) && within(pos[2], box[4], box[5]) &&
            within(pos[3], box[6], box[7])) {
            found = at<u32>(box, 0x20);
            break;
        }
    }
    return found ? found : at<u32>(boxes, count * 0x2c - 0xc);
}

void __cdecl Coaster_BuildNodes(u8 *coaster) {
    at<u32>(coaster, 0xa0) = 0;
    if (!at<u8 *>(*reinterpret_cast<u8 *const *>(0x00790fe4), 0x18))
        return;
    u32 cursor[4];
    Begin(cursor);
    Vec3 pos;
    TrackSpline_Point(ListNode(cursor[1], 0), ListNode(cursor[3], cursor[2]), 0.0f, &pos);   // (sic: no end piece faults)
    const float spacing = at<float>(coaster, 0xac);

    u32 estimate = 0;   // (the summed distance beside it is never read)
    {
        u32 sizing[4];
        Begin(sizing);
        Vec3 last;
        TrackSpline_Point(ListNode(sizing[1], 0), ListNode(sizing[3], sizing[2]), 0.0f, &last);
        while (sizing[3]) {
            Vec3 next;
            TrackCursor_Point(sizing, 1.0f, &next);
            const X87Reg d = Vec3_Distance(&last.x, &next.x);
            estimate += (u32)(s32)(long long)X87_SubR(d / spacing * kMinusThree, 1.0f).v;   // __ftol
            TrackCursor_Next(sizing);
            last = next;
        }
    }
    if (estimate > at<u32>(coaster, 0x98)) {
        GlobalMem_Free(at<void *>(coaster, 0x9c));
        const u32 capacity = (u32)(s32)(long long)X87_MulInt(estimate, 1.1f);
        at<u32>(coaster, 0x98) = capacity;
        at<void *>(coaster, 0x9c) = GlobalMem_Alloc(capacity * 0x38, 1);
    }

    u8 *out = at<u8 *>(coaster, 0x9c);
    float t = kStep, before = 0.0f, sum = 0.0f, sumBefore = 0.0f;
    u8 *node = ListNode(cursor[1], cursor[0]);
    u8 *node2 = ListNode(cursor[1], cursor[0], 8);
    at<u32>(node, 8) = 0;
    at<u32>(node2, 8) = 0;
    bool first = true, more = true;
    for (;;) {
        if (!cursor[3] && !more)
            break;
        Vec3 q;
        TrackSpline_Point(ListNode(cursor[1], cursor[0]), cursor[3] ? ListNode(cursor[3], cursor[2]) : WrapNode(coaster), t, &q);
        const X87Reg dx = X87Reg(pos.x) - q.x;
        const float dy = (float)(X87Reg(pos.y) - q.y), dz = (float)(X87Reg(pos.z) - q.z);
        const X87Reg dist = Length(dx, dy, dz);
        const float distance = (float)dist;
        sum = (float)(dist + sum);
        bool record = true;
        X87Reg f = 0.0f;
        if (first) {
            first = false;
        } else if (!(sum >= spacing)) {   // (fcomp: below or unordered)
            before = t;
            t = (float)(X87Reg(kStep) + t);
            pos = q;
            record = false;
        } else {
            f = (X87Reg(spacing) - sumBefore) / distance;
        }
        if (record) {
            const X87Reg g = X87Reg(1.0f) - f;
            pos.x = (float)(X87Reg(pos.x) * g + f * q.x);
            pos.y = (float)(X87Reg(pos.y) * g + f * q.y);
            pos.z = (float)(X87Reg(pos.z) * g + f * q.z);
            before = (float)(X87Reg(before) * g + f * t);
            const X87Reg r = std::fmod((double)before, (double)kStep);   // _CIfmod
            sum = 0.0f;
            float *const point = &at<float>(out, 4);
            point[0] = pos.x;
            point[1] = pos.y;
            point[2] = pos.z;
            t = (float)(X87_SubR(r, kStep) + before);
            u8 *&at2c = at<u8 *>(out, 0x2c);
            u8 *&at30 = at<u8 *>(out, 0x30);
            at2c = ListNode(cursor[1], cursor[0]);
            at30 = ListNode(cursor[1], cursor[0], 8);
            float &along = at<float>(out, 0x34);
            along = before;
            if (!(along >= 0.0f)) {
                X87Reg v = 0.0;
                do {
                    if (at<u8 *>(at2c, 0x14))
                        at2c = at<u8 *>(at2c, 0x14);
                    if (at<u8 *>(at30, 0x14))
                        at30 = at<u8 *>(at30, 0x14);
                    v = X87Reg(along) - kMinusOne;
                    along = (float)v;
                } while (!(v.v >= 0.0));
            }
            const s32 whole = (s32)(long long)at<float>(at2c, 8);   // __ftol
            if (whole && whole == (s32)(out - at<u8 *>(coaster, 0x9c)) / 0x38) {
                at<float>(at2c, 8) = (float)(X87Reg((double)(u32)whole) - 0.001f);
                at<u32>(at30, 8) = at<u32>(at2c, 8);
            }
            TrackOrientation o;
            std::memset(&o, 0, sizeof o);
            const u32 max = 0x7f7fffff;   // FLT_MAX
            std::memcpy(&o.rawYaw, &max, 4);
            Vec3 tangent;
            if (cursor[3]) {
                TrackSpline_Tangent(at2c, at<u8 *>(at2c, 0x18), along, &tangent);
                TrackOrientation_Set(tangent, bits(along), &o, nullptr);
                Vec3 rel;
                TrackSpline_Point(at30, at<u8 *>(at30, 0x18), along, &rel);
                rel.x = (float)(X87Reg(rel.x) - point[0]);
                rel.y = (float)(X87Reg(rel.y) - point[1]);
                rel.z = (float)(X87Reg(rel.z) - point[2]);
                const SinTable sines = SinTable_Game();
                {   // turned back by the yaw about y: x c + z s, z c - x s
                    const float angle = -o.yaw;
                    const X87Reg s = SinTable_At(sines, (float)(X87Reg(sines.scale) * angle));
                    const X87Reg c = SinTable_At(sines, (float)((X87Reg(angle) - kMinusHalfPi) * sines.scale));
                    const float x = rel.x, z = rel.z;
                    rel.x = (float)(X87Reg(x) * c + X87Reg(z) * s);
                    rel.z = (float)(X87Reg(z) * c - X87Reg(x) * s);
                }
                {   // and by the pitch about x: y c - z s, z c + y s
                    const float angle = -o.pitch;
                    const X87Reg s = SinTable_At(sines, (float)(X87Reg(sines.scale) * angle));
                    const X87Reg c = SinTable_At(sines, (float)((X87Reg(angle) - kMinusHalfPi) * sines.scale));
                    const float y = rel.y, z = rel.z;
                    rel.y = (float)(X87Reg(y) * c - X87Reg(z) * s);
                    rel.z = (float)(X87Reg(z) * c + X87Reg(y) * s);
                }
                o.roll = (float)Vec3_AngleZ(&rel.x);
            } else {
                TrackSpline_Tangent(at2c, WrapNode(coaster), along, &tangent);
                TrackOrientation_Set(tangent, bits(along), &o, nullptr);
            }
            std::memcpy(&at<u32>(out, 0x10), &o.yaw, 12);
            if (!cursor[3])
                more = false;
            out += 0x38;
        }

        if (t > 1.0f) {   // (fcomp: above)
            if (cursor[3]) {
                TrackCursor_Next(cursor);
                before = (float)(X87Reg(before) - 1.0f);
                t = (float)(X87Reg(t) - 1.0f);
                node = ListNode(cursor[1], cursor[0]);
                node2 = ListNode(cursor[1], cursor[0], 8);
                float d[3] = {(float)(X87Reg(at<float>(node, 0x40)) - pos.x), (float)(X87Reg(at<float>(node, 0x44)) - pos.y),
                              (float)(X87Reg(at<float>(node, 0x48)) - pos.z)};
                if (cursor[3]) {   // the way to the node along the spline's direction there
                    Vec3 ahead;
                    TrackSpline_Point(ListNode(cursor[1], cursor[0]), ListNode(cursor[3], cursor[2]), t, &ahead);
                    const X87Reg ex = X87Reg(ahead.x) - pos.x;
                    const float ey = (float)(X87Reg(ahead.y) - pos.y), ez = (float)(X87Reg(ahead.z) - pos.z);
                    const float el = (float)Length(ex, ey, ez);
                    const float e[3] = {(float)(ex / el), (float)(X87Reg(ey) / el), (float)(X87Reg(ez) / el)};
                    const X87Reg dl = Length(d[0], d[1], d[2]);
                    const float u[3] = {(float)(X87Reg(d[0]) / dl), (float)(X87Reg(d[1]) / dl), (float)(X87Reg(d[2]) / dl)};
                    if (u[0] == u[0]) {   // _isnan
                        const X87Reg dot = X87Reg(u[0]) * e[0] + X87Reg(u[1]) * e[1] + X87Reg(u[2]) * e[2];
                        for (float &v : d)
                            v = (float)(X87Reg(v) * dot);
                    }
                }
                const X87Reg v = (Length(d[0], d[1], d[2]) + sum) / at<float>(coaster, 0xac);
                const s32 n = (s32)(out - at<u8 *>(coaster, 0x9c)) / 0x38;
                at<float>(node, 8) = n ? (float)(X87Reg((double)(u32)(n - 1)) + v) : (float)v;
                at<u32>(node2, 8) = at<u32>(node, 8);
                if (!cursor[3])
                    at<u32>(coaster, 0xa8) = at<u32>(node, 8);
                u8 *const owner = at<u8 *>(node, 0x10);
                const u32 *const link = at<const u32 *>(owner, 0x74);
                if (!link || (at<u32>(owner, 8) & 0x2000) || *link == cursor[0])
                    at<s32>(owner, 0xfc) = (s32)(long long)at<float>(node, 8);   // __ftol
            }
        } else if (!(t >= 0.0f) && cursor[3]) {   // (fcomp: below or unordered)
            TrackCursor_Prev(cursor);
            before = (float)(X87Reg(before) - kMinusOne);
            t = (float)(X87Reg(t) - kMinusOne);
        }
        sumBefore = sum;
    }

    at<u32>(coaster, 0xa0) = 0;
    u8 *prev = at<u8 *>(*reinterpret_cast<u8 *const *>(0x00790fe4), 0x18);
    for (u8 *p = at<u8 *>(prev, 0x64); p; prev = p, p = at<u8 *>(p, 0x64)) {
        const u32 span = at<u32>(p, 0xfc) - at<u32>(prev, 0xfc) + 1;
        if (span > at<u32>(coaster, 0xa0))
            at<u32>(coaster, 0xa0) = span;
    }
    at<s32>(coaster, 0xa4) = (s32)(out - at<u8 *>(coaster, 0x9c)) / 0x38;
    std::memcpy(out, out - 0x38, 0x38);
    double whole;
    const double frac = std::modf((double)at<float>(coaster, 0xa8), &whole);
    s32 count = (s32)(long long)whole;   // __ftol
    if (frac < 0.0 || frac > 0.0)        // (fcomp 0: a NaN counts as equal)
        ++count;
    if (count >= at<s32>(coaster, 0xa4))
        ++at<s32>(coaster, 0xa4);
    Coaster_SetSegmentWindow(coaster);
}

void __cdecl CoasterSearch_Reset(u32 *search) {
    for (int i = 0; i < 7; ++i) search[i] = 0;
    search[7] = 0x7f7fffff;
}

double __cdecl CoasterNodes_SplineAt(const u8 *nodes, float t) {
    s32 index;
    Float_SplitWhole(t, &index);
    const u8 *node = at<const u8 *>(nodes, 0x2c + (u32)index * 0x38);
    for (const u8 *next; (next = at<const u8 *>(node, 0x18)) && !(at<float>(next, 8) >= t);)   // (fcomp; test ah, 1: a NaN walks on)
        node = next;
    const u8 *const next = at<const u8 *>(node, 0x18);
    const X87Reg way = next ? (X87Reg(t) - at<float>(node, 8)) / (X87Reg(at<float>(next, 8)) - at<float>(node, 8)) : X87Reg(0.0f);   // 0x6fe10c
    return (way + X87Reg((double)at<s32>(node, 0))).v;   // fiadd
}
}
