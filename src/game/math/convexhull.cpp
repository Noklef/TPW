#include "convexhull.h"

#include <cstdint>
#include <cstring>

#include "../core/movs.h"
#include "x87.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

// The constants, as the game's memory holds them: 0, 1.
const float &kZero = global<float>(0x007014a0);
const float &kOne = global<float>(0x00701490);

const u32 kPoint = 0xc;

s32 &Count(u8 *hull) { return at<s32>(hull, 0xd0); }
s32 &Outside(u8 *hull) { return at<s32>(hull, 0xd4); }
s8 &Flag(u8 *hull, s32 k) { return at<s8>(hull, 0xc0 + k); }
u8 *Point(u8 *hull, s32 k) { return hull + (u32)k * kPoint; }
float X(const u8 *point) { return at<float>(point, 0); }
float Z(const u8 *point) { return at<float>(point, 8); }

void Put(u8 *to, u32 x, u32 y, u32 z) {
    at<u32>(to, 0) = x;
    at<u32>(to, 4) = y;
    at<u32>(to, 8) = z;
}
void Copy(u8 *to, const u8 *from) { Put(to, at<u32>(from, 0), at<u32>(from, 4), at<u32>(from, 8)); }

float AsFloat(const u32 &bits) { return reinterpret_cast<const float &>(bits); }

// FCOM's "equal", which a NaN is too.
bool Ties(float a, float b) { return !(a < b) && !(a > b); }

// The new point's side of the edge from `before` to `point`, over 0 outside.
X87Reg Side(const u8 *before, const u8 *point, float x, float z) {
    const float dz = (float)X87Reg(-(X87Reg(Z(point)) - Z(before)).v);
    const float dx = (float)(X87Reg(X(point)) - X(before));
    return (X87Reg(z) - Z(before)) * dx + (X87Reg(x) - X(before)) * dz;
}

// Two points and a third.
void AddThird(u8 *hull, u32 xBits, u32 y, u32 zBits) {
    const float x = AsFloat(xBits), z = AsFloat(zBits);
    u8 *const first = Point(hull, 0), *const second = Point(hull, 1);
    const float side = (float)Side(first, second, x, z);
    if (side > kZero) {
        Copy(Point(hull, Count(hull)), second);
        ++Count(hull);
        Put(second, xBits, y, zBits);
        return;
    }
    if (!(side >= kZero)) {
        Put(Point(hull, Count(hull)), xBits, y, zBits);
        ++Count(hull);
        return;
    }
    const float dx = (float)(X87Reg(X(second)) - X(first));
    float along;
    if (Ties(dx, kZero)) {
        const float dz = (float)(X87Reg(Z(second)) - Z(first));
        along = (float)((X87Reg(z) - Z(first)) / dz);
    } else {
        along = (float)((X87Reg(x) - X(first)) / dx);
    }
    if (along > kOne)
        Put(second, xBits, y, zBits);
    else if (!(along >= kZero))
        Put(first, xBits, y, zBits);
}

// More than one edge has the new point outside.
void Rebuild(u8 *hull, u32 x, u32 y, u32 z) {
    s32 k = Count(hull) - 1;
    while (Flag(hull, k) == 0 && k >= 0)
        --k;
    while (Flag(hull, k) != 0 && k >= 0)
        --k;
    ++k;
    Rep_Movs(&Flag(hull, Count(hull)), &Flag(hull, 0), (u32)k);
    Rep_Movs(&Flag(hull, 0), &Flag(hull, k), (u32)Count(hull));
    Rep_Movs(Point(hull, Count(hull)), hull, (u32)k * kPoint);
    Rep_Movs(hull, Point(hull, k), (u32)Count(hull) * kPoint);

    u8 saved[0xc0];
    Rep_Movs(saved, hull, (u32)Count(hull) * kPoint);
    s32 last = Outside(hull) - 1;
    if (last > Count(hull))
        last -= Count(hull);
    if (last == 0)
        while (Flag(hull, last + 1) == 0)
            ++last;
    Put(hull, x, y, z);
    Copy(Point(hull, 1), saved + (u32)last * kPoint);

    s32 next = 1, out = 2;
    if (next == Count(hull))
        next = 0;
    while (next != 0) {
        if (Flag(hull, next) == 0) {
            Copy(Point(hull, out), saved + (u32)next * kPoint);
            ++out;
        }
        ++next;
        if (next == Count(hull))
            next = 0;
    }
    Count(hull) = out;
}

}  // namespace

void __fastcall ConvexHull_AddPoint(u8 *hull, void *, u32 xBits, u32 y, u32 zBits) {
    const float x = AsFloat(xBits), z = AsFloat(zBits);
    if (Count(hull) < 3) {
        if (Count(hull) == 0) {
            Put(Point(hull, 0), xBits, y, zBits);
            ++Count(hull);
        } else if (Count(hull) == 1) {
            if (!(Ties(x, X(hull)) && Ties(z, Z(hull)))) {
                Put(Point(hull, 1), xBits, y, zBits);
                ++Count(hull);
            }
        } else {
            AddThird(hull, xBits, y, zBits);
        }
        return;
    }

    s32 any = 0;
    Outside(hull) = 0;
    for (s32 i = Count(hull); i--;) {
        s32 before = i - 1;
        if (before < 0)
            before = Count(hull) - 1;
        Flag(hull, i) = Side(Point(hull, before), Point(hull, i), x, z).v > kZero ? 1 : 0;
        Outside(hull) += Flag(hull, i);
        any |= Flag(hull, i);
    }
    if (!any)
        return;
    s32 first = 0;
    while (Flag(hull, first) == 0)
        ++first;
    if (Outside(hull) > 1) {
        Rebuild(hull, xBits, y, zBits);
    } else {
        std::memmove(Point(hull, first + 1), Point(hull, first), (u32)(Count(hull) - first) * kPoint);  // memmove (0x67bb90)
        Put(Point(hull, first), xBits, y, zBits);
        ++Count(hull);
    }
}
