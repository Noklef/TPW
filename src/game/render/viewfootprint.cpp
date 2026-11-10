#include "viewfootprint.h"

#include <cstdint>
#include <cstring>

#include "../core/crt.h"
#include "../math/convexhull.h"
#include "../math/rectclip.h"
#include "../math/x87.h"
#include "viewquad.h"
#include "worldview.h"

namespace {

AddrCalls g_calls;

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

// __ftol's low half.
s32 Ftol(X87Reg v) { return (s32)(u32)(long long)v.v; }

// A scan table's cell of a row, first (+8) or last (+0x88).
s8 &Cell(u8 *scan, u32 side, s32 row) { return at<s8>(scan, (u32)(side + row)); }

void First(u8 *scan, s32 row, s32 cell) {
    if (Cell(scan, 8, row) > cell) Cell(scan, 8, row) = (s8)cell;
}
void Last(u8 *scan, s32 row, s32 cell) {
    if (Cell(scan, 0x88, row) < cell) Cell(scan, 0x88, row) = (s8)cell;
}

void Rows(u8 *scan, s32 top, s32 bottom) {
    if (top < at<s32>(scan, 0)) at<s32>(scan, 0) = top;
    if (bottom > at<s32>(scan, 4)) at<s32>(scan, 4) = bottom;
}

}  // namespace

void ViewFootprint_SetCalls(const AddrCall *table, u32 count) { g_calls.Set(table, count); }

extern "C" {

void __fastcall Polygon_ScanRows(const u8 *polygon, void *, u8 *scan, u8 *out, u32, s32 limit) {
    std::memset(scan + 8, 0x7f, 0x80);
    std::memset(scan + 0x88, 0, 0x80);
    at<s32>(scan, 0) = 0x7f;
    at<s32>(scan, 4) = 0;
    const float half = global<float>(0x0070148c);   // (just under 1: rounds up what is not a whole cell)
    const s32 count = at<s32>(polygon, 0xd0);
    s32 previous = count - 1;
    for (s32 i = 0; i < count; previous = i, ++i) {
        const float xi = at<float>(polygon, (u32)i * 12), zi = at<float>(polygon, (u32)i * 12 + 8);
        const float xp = at<float>(polygon, (u32)previous * 12), zp = at<float>(polygon, (u32)previous * 12 + 8);
        if (zi > zp) {
            const s32 from = Ftol(xp), to = Ftol(xi), top = Ftol(zp), bottom = Ftol(X87Reg(zi) + half);
            const float slope = (float)((X87Reg(xi) - xp) / (X87Reg(zi) - zp));
            float x = xp;
            if (Ftol(zi) > Ftol(zp)) x = (float)((X87Reg((double)Ftol(X87Reg(zp) + half)) - zp) * slope + x);
            Rows(scan, top, bottom);
            First(scan, top, from);
            First(scan, bottom, to);
            First(scan, top, Ftol(x));
            for (s32 row = top + 1; row < bottom; ++row) {
                First(scan, row, Ftol(x));
                if (row < bottom - 1) {
                    x = (float)(X87Reg(x) + slope);
                    First(scan, row, Ftol(x));
                }
            }
            First(scan, bottom, Ftol(x));
        } else if (!(zi >= zp)) {   // (below, or a NaN)
            const s32 from = Ftol(X87Reg(xp) + half), to = Ftol(X87Reg(xi) + half), bottom = Ftol(X87Reg(zp) + half), top = Ftol(zi);
            const float slope = (float)((X87Reg(xp) - xi) / (X87Reg(zp) - zi));
            float x = (float)(X87Reg(xi) + half);
            if (Ftol(zi) < Ftol(zp)) x = (float)((X87Reg((double)Ftol(X87Reg(zi) + half)) - zi) * slope + x);
            Rows(scan, top, bottom);
            Last(scan, bottom, from);
            Last(scan, top, to);
            Last(scan, top, Ftol(x));
            for (s32 row = top + 1; row < bottom; ++row) {
                Last(scan, row, Ftol(x));
                if (row < bottom - 1) {
                    x = (float)(X87Reg(x) + slope);
                    Last(scan, row, Ftol(x));
                }
            }
            Last(scan, bottom, Ftol(x));
        }
    }
    for (s32 row = at<s32>(scan, 0); row <= at<s32>(scan, 4); ++row)
        if (Cell(scan, 0x88, row) < Cell(scan, 8, row)) Cell(scan, 0x88, row) = Cell(scan, 8, row);
    if (global<u32>(0x008bd508) & 0x1000000) {
        char line[200];
        for (s32 i = 0; i < at<s32>(polygon, 0xd0); ++i) {
            Crt().sprintf(line, gText_Polygon_ScanRows_76830c, (double)at<float>(polygon, (u32)i * 12), (double)at<float>(polygon, (u32)i * 12 + 8));
            g_calls.Cd(0x00564650, line);
        }
    }
    for (s32 row = at<s32>(scan, 0); row <= at<s32>(scan, 4); ++row) {
        Cell(out, 8, row) = Cell(scan, 8, row);
        if (Cell(scan, 8, row) > Cell(scan, 8, row + 1)) Cell(out, 8, row) = Cell(scan, 8, row + 1);
        Cell(out, 0x88, row) = Cell(scan, 0x88, row);
        if (Cell(scan, 0x88, row) < Cell(scan, 0x88, row + 1)) Cell(out, 0x88, row) = Cell(scan, 0x88, row + 1);
    }
    at<s32>(out, 0) = at<s32>(scan, 0);
    at<s32>(out, 4) = at<s32>(out, 4) > limit ? limit : at<s32>(scan, 4);
}

u32 __fastcall WorldView_UpdateFootprint(u8 *view) {
    u8 *const scan = view + 0xc4;
    at<s32>(scan, 0) = 0x7f;
    at<s32>(scan, 4) = -1;
    std::memset(scan + 8, 0x7f, 0x80);
    std::memset(scan + 0x88, 0xff, 0x80);
    ViewQuad_SetScaled(reinterpret_cast<float *>(view), global<float>(0x0087af58), 1.0f);
    u8 polygon[0xd4];
    const u8 *const camera = global<const u8 *>(0x0087afe8);
    const float eye[3] = {at<float>(camera, 0x30), at<float>(camera, 0x34), at<float>(camera, 0x38)};
    at<s32>(polygon, 0xd0) = 1;
    std::memcpy(polygon, eye, 12);
    float far_ = 10.0f;
    for (u32 corner = 0; corner < 4; ++corner) {
        float point[3];
        WorldView_RayPoint(view, nullptr, point, reinterpret_cast<const float *>(view + corner * 12));
        ConvexHull_AddPoint(polygon, nullptr, at<u32>(point, 0), at<u32>(point, 4), at<u32>(point, 8));
        const X87Reg dx = X87Reg(point[0]) - eye[0], dy = X87Reg(point[1]) - eye[1], dz = X87Reg(point[2]) - eye[2];
        const X87Reg length = X87_Sqrt(dx * dx + dy * dy + dz * dz);
        if (length.v > far_) far_ = (float)length;
    }
    Polygon_ClipToRect(polygon, nullptr, at<float>(view, 0x3c), (float)(X87Reg(at<float>(view, 0x40)) - at<float>(view, 0x4c)), at<float>(view, 0x44), (float)(X87Reg(at<float>(view, 0x48)) - at<float>(view, 0x50)));
    const float perZ = (float)(X87Reg(global<float>(0x00701490)) / at<float>(view, 0x50));
    const float perX = (float)(X87Reg(global<float>(0x00701490)) / at<float>(view, 0x4c));
    for (s32 i = 0; i < at<s32>(polygon, 0xd0); ++i) {
        float &x = at<float>(polygon, (u32)i * 12), &z = at<float>(polygon, (u32)i * 12 + 8);
        x = (float)(X87Reg(perX) * x);
        z = (float)(X87Reg(perZ) * z);
    }
    Polygon_ScanRows(polygon, nullptr, scan, view + 0x1cc, 0, at<s32>(view, 0x58));
    u32 farBits;
    std::memcpy(&farBits, &far_, 4);
    g_calls.Th(0x00578ca0, 0x0087b110u, farBits);
    return 1;
}
}
