#include "ribbon.h"

#include <cstdint>

#include "../math/mat4.h"
#include "../math/x87.h"
#include "drawbuffer.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const RibbonEnv kPortEnv = {&DrawBatch_AddPolygon};
const RibbonEnv *g_env = &kPortEnv;

const u32 kStride = 0x20;   // [0x701c04]

// A point's four dwords (x and y read as floats, z and w as they are).
struct Point {
    u32 d[4];
    float x() const { return *reinterpret_cast<const float *>(&d[0]); }
    float y() const { return *reinterpret_cast<const float *>(&d[1]); }
    void setX(float v) { *reinterpret_cast<float *>(&d[0]) = v; }
    void setY(float v) { *reinterpret_cast<float *>(&d[1]) = v; }
};

// fcom's C3: equal or unordered.
bool level(double a, double b) { return !(a < b) && !(a > b); }
bool nothing(X87Reg sum) { return level(sum.v, 0.0); }

// The width over the length and the count (fdivr of the width's float).
X87Reg scale(const u8 *object, X87Reg product) { return X87Reg(at<float>(object, 0xa16c)) / product; }

void vertex(u8 *v, const Point &p, u32 colour, u32 u, u32 w, const u8 *object) {
    at<Point>(v, 0) = p;
    at<u32>(v, 0x10) = colour;
    at<float>(v, 0x18) = X87_Through(at<float>(object, u));
    at<float>(v, 0x1c) = X87_Through(at<float>(object, w));
}

// The ribbon's first point: its place, sideways and edges.
void start(u8 *object, u32 ribbon, const float *from, const float *to) {
    const u32 in[6] = {at<u32>(from, 0), at<u32>(from, 4), at<u32>(from, 8), at<u32>(to, 0), at<u32>(to, 4), at<u32>(to, 8)};
    Point out[2];
    u8 *const projection = global<u8 *>(0x0087b1d8);
    const auto project = at<void(__cdecl *)(u8 *, u32, Point *, const u32 *, u32, u32, u32, u32)>(at<const u8 *>(projection, 0), 4);
    project(projection, global<u32>(0x0087b088), out, in, 0x10, 0xc, 2, 0);
    at<Point>(object, 0x9de4 + ribbon * 0x10) = out[1];
    float &sideX = at<float>(object, 0xa0e4 + ribbon * 8), &sideY = at<float>(object, 0xa0e8 + ribbon * 8);
    sideX = (float)(X87Reg(out[0].x()) - out[1].x());
    sideY = (float)(X87Reg(out[0].y()) - out[1].y());
    const X87Reg sum = X87Reg(sideY) * X87Reg(sideY) + X87Reg(sideX) * X87Reg(sideX);
    if (nothing(sum)) return;
    const X87Reg k = scale(object, X87_Sqrt(sum) * X87Reg((s32)(ribbon + 1)));
    sideX = (float)(k * sideX);
    sideY = (float)(k * sideY);
    Point &left = at<Point>(object, 0x9ee4 + ribbon * 0x20), &right = at<Point>(object, 0x9ef4 + ribbon * 0x20);
    right = out[1];
    left = right;
    left.setX((float)(X87Reg(sideY) + left.x()));
    left.setY((float)(X87Reg(left.y()) - sideX));
    right.setX((float)(X87Reg(right.x()) - sideY));
    right.setY((float)(X87Reg(sideX) + right.y()));
}

// A later point with from: the sum of the sideways and the scaled step gives the new edges; all kept. False: nothing to draw.
bool extend(u8 *object, u32 ribbon, const Point &place, Point &left, Point &right) {
    Point &last = at<Point>(object, 0x9de4 + ribbon * 0x10);
    const X87Reg stepX = X87Reg(place.x()) - last.x(), stepY = X87Reg(place.y()) - last.y();
    const float stepXf = (float)stepX, stepYf = (float)stepY;
    const X87Reg sum = stepY * stepYf + stepX * stepXf;
    if (nothing(sum)) return false;
    const X87Reg n = X87Reg((s32)(ribbon + 1));
    const X87Reg k = scale(object, X87_Sqrt(sum) * n);
    const float kf = (float)k;
    const float sideX = (float)(k * stepXf), sideY = (float)(X87Reg(kf) * stepYf);
    const X87Reg sumX = X87Reg(at<float>(object, 0xa0e4 + ribbon * 8)) + sideX, sumY = X87Reg(at<float>(object, 0xa0e8 + ribbon * 8)) + sideY;
    const float sumXf = (float)sumX, sumYf = (float)sumY;
    const X87Reg sum2 = sumY * sumYf + sumX * sumXf;
    if (nothing(sum2)) return false;
    const X87Reg k2 = scale(object, n * X87_Sqrt(sum2));
    const float ux = (float)(k2 * sumXf);
    const X87Reg uy = k2 * sumYf;
    left = last;
    right = last;
    left.setX((float)(uy + last.x()));
    left.setY((float)(X87Reg(last.y()) - ux));
    right.setX((float)(X87Reg(last.x()) - uy));
    right.setY((float)(X87Reg(ux) + last.y()));
    at<float>(object, 0xa0e4 + ribbon * 8) = sideX;
    at<float>(object, 0xa0e8 + ribbon * 8) = sideY;
    last = place;
    at<Point>(object, 0x9ee4 + ribbon * 0x20) = left;
    at<Point>(object, 0x9ef4 + ribbon * 0x20) = right;
    return true;
}

// A later point without from: the edges at the new point, the step scaled and turned; nothing kept. False: nothing to draw.
bool loose(const u8 *object, u32 ribbon, const Point &place, Point &left, Point &right) {
    const Point &last = at<Point>(object, 0x9de4 + ribbon * 0x10);
    const float stepX = (float)(X87Reg(place.x()) - last.x());
    const X87Reg stepY = X87Reg(place.y()) - last.y();
    const float stepYf = (float)stepY;
    const X87Reg sum = stepY * stepYf + X87Reg(stepX) * stepX;
    if (nothing(sum)) return false;
    const X87Reg k = scale(object, X87_Sqrt(sum) * X87Reg((s32)(ribbon + 1)));
    const X87Reg a = k * stepX, b = X87Reg((float)k) * stepYf;
    const float af = (float)a, bf = (float)b;
    left = place;
    right = place;
    left.setY((float)(X87Reg(place.y()) - af));
    right.setY((float)(a + place.y()));
    left.setX((float)(b + place.x()));
    right.setX((float)(X87Reg(place.x()) - bf));
    return true;
}

}  // namespace

void Ribbon_SetEnv(const RibbonEnv *env) { g_env = env ? env : &kPortEnv; }

extern "C" {

u32 __fastcall Ribbon_AddPoint(u8 *object, void *, const float *from, const float *to) {
    if (from && to && level((double)from[0], (double)to[0]) && level((double)from[1], (double)to[1]) && level((double)from[2], (double)to[2])) return 1;
    const u32 ribbon = at<u32>(object, 0xa188);
    u32 &count = at<u32>(object, 0x9da4 + ribbon * 4);
    const u32 points = count++;
    if (!points) {
        if (from) start(object, ribbon, from, to);
        return 1;
    }

    Point place;
    Mat4_Transform(global<const float *>(0x0087b088), nullptr, reinterpret_cast<float *>(&place), to, 0);
    const Point oldLeft = at<Point>(object, 0x9ee4 + ribbon * 0x20), oldRight = at<Point>(object, 0x9ef4 + ribbon * 0x20);
    Point left, right;
    if (!(from ? extend(object, ribbon, place, left, right) : loose(object, ribbon, place, left, right))) return 1;

    const u32 colour = at<u32>(object, 0xa19c);
    u8 vertices[20 * kStride];   // (room as in the original: clipping swaps its output with this buffer)
    vertex(vertices, oldLeft, colour, 0xa170, 0xa174, object);
    vertex(vertices + kStride, oldRight, colour, 0xa178, 0xa174, object);
    vertex(vertices + 2 * kStride, right, colour, 0xa178, 0xa17c, object);
    vertex(vertices + 3 * kStride, left, colour, 0xa170, 0xa17c, object);
    return g_env->addPolygon(global<u8 *>(0x00879ab0), nullptr, vertices, 4, kStride, nullptr, 0) != 0 ? 1 : 0;
}
}
