#include "raypick.h"

#include <emmintrin.h>

#include <cmath>
#include <cstdint>

#include "../math/matrix.h"
#include "../math/x87.h"
#include "../coaster/train.h"
#include "../model/highlight.h"
#include "../model/worldverts.h"
#include "../motion/normals.h"
#include "land.h"
#include "../math/sintable.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const float kFullCircle = 6.2821855545043945f;   // 0x6fe678
const float kOne = 1.0f;                          // 0x6fe620
const float kZero = 0.0f;                         // 0x6fe630
const float kTenth = 0.100000001f;                // 0x6fe664
const float kHalf = 0.5f;                         // 0x6fe634
const float kQuarterTurn = -1.57079637f;          // 0x6fe638
const double kTurnDown = -1.5707963705062866;     // 0x6fe648
const double kTurnUp = 1.5707963705062866;        // 0x6fe650
const float kPastCell = 1.00999999f;              // 0x6fe668
const float kStep = 10.0f;

const RayPickEnv kPortEnv = {&WorldVerts_TransformNode, &Terrain_SurfaceHeight, &Ride_Train, &Train_CarValue, &Model_UpdateNormals, &RayPick_Node, &RayPick_Column, &Highlight_SetTarget};

s32 Fistp(float x) { return _mm_cvtss_si32(_mm_set_ss(x)); }
s32 Ftol(double x) { return (s32)(long long)x; }
bool Below(double v) { return !(v >= kZero); }
bool Above(double v) { return v > kZero; }
bool Crosses(double a, double b) { return (Below(a) && Above(b)) || (Above(a) && Below(b)); }

// About the z axis, as the x87 has it: the cosine in the register, the sine stored.
void TurnZ(float *v, X87Reg angle) {
    const float s = (float)X87_Sin(angle.v);
    const X87Reg c(X87_Cos(angle.v));
    const float x = v[0];
    v[0] = (float)((c * x) - (X87Reg(s) * v[1]));
    v[1] = (float)((c * v[1]) + (X87Reg(s) * x));
}

s32 Clamped(s32 v, u32 count) {
    if (v < 0) v = 0;
    if (!((u32)v < count)) v = (s32)(count - 1);
    return v;
}
const RayPickEnv *g_env = &kPortEnv;

// The game's acos (0x67b4e0) of a register, as in motion/quat.cpp. Past 1 (a dot of rounded unit vectors) it gives a NaN, so no hit, here as there; there the CRT's math error path also sets
// errno and raises a masked exception, not kept here.
double CrtAcos(X87Reg x) {
    const X87Reg r = X87_Sqrt((X87Reg(1.0) + x) * (X87Reg(1.0) - x));
    return X87_Atan2(r.v, x.v);
}

X87Reg Dot(const float *a, const float *b) { return ((X87Reg(a[1]) * b[1]) + (X87Reg(a[2]) * b[2])) + (X87Reg(b[0]) * a[0]); }

// Face against the ray: the point stored and true when it is in the face.
bool HitFace(const float *v1, const float *v2, const float *v3, const float *normalIn, const float *m) {
    float n[3] = {normalIn[0], normalIn[1], normalIn[2]};
    Matrix_TransformVector(m, n);
    const X87Reg length = X87_Sqrt(((X87Reg(n[0]) * X87Reg(n[0])) + (X87Reg(n[1]) * n[1])) + (X87Reg(n[2]) * n[2]));
    n[0] = (float)(X87Reg(n[0]) / length);
    n[1] = (float)(X87Reg(n[1]) / length);
    n[2] = (float)(X87Reg(n[2]) / length);

    const float *o = gRaypick_7a1cb0;
    const float *d = gRaypick_7a1cd8;
    const X87Reg p = ((X87Reg(o[0]) * n[0]) + (X87Reg(n[1]) * o[1])) + (X87Reg(n[2]) * o[2]);
    const X87Reg q = ((X87Reg(n[2]) * v1[2]) + (X87Reg(n[1]) * v1[1])) + (X87Reg(n[0]) * v1[0]);
    const X87Reg top = p + X87Reg(-q.v);
    const X87Reg under = ((X87Reg(d[0]) * n[0]) + (X87Reg(n[1]) * d[1])) + (X87Reg(n[2]) * d[2]);
    const X87Reg back(-(top / under).v);

    float h[3];
    h[0] = (float)((X87Reg(d[0]) * back) + o[0]);
    h[1] = (float)((back * d[1]) + o[1]);
    h[2] = (float)((back * d[2]) + o[2]);

    const X87Reg ax = X87Reg(v1[0]) - h[0];
    float a[3] = {(float)ax, (float)(X87Reg(v1[1]) - h[1]), (float)(X87Reg(v1[2]) - h[2])};
    float b[3], c[3];
    for (int i = 0; i < 3; ++i) {
        b[i] = (float)(X87Reg(v2[i]) - h[i]);
        c[i] = (float)(X87Reg(v3[i]) - h[i]);
    }
    const float lengthA = (float)X87_Sqrt(((ax * ax) + (X87Reg(a[1]) * a[1])) + (X87Reg(a[2]) * a[2]));
    const float lengthB = (float)X87_Sqrt(((X87Reg(b[0]) * b[0]) + (X87Reg(b[1]) * b[1])) + (X87Reg(b[2]) * b[2]));
    const float lengthC = (float)X87_Sqrt(((X87Reg(c[0]) * c[0]) + (X87Reg(c[1]) * c[1])) + (X87Reg(c[2]) * c[2]));
    for (int i = 0; i < 3; ++i) {
        a[i] = (float)(X87Reg(a[i]) / lengthA);
        b[i] = (float)(X87Reg(b[i]) / lengthB);
        c[i] = (float)(X87Reg(c[i]) / lengthC);
    }

    const float first = (float)CrtAcos(Dot(c, b));
    const double second = CrtAcos(((X87Reg(b[0]) * a[0]) + (X87Reg(b[1]) * a[1])) + (X87Reg(b[2]) * a[2]));
    const X87Reg sum = X87Reg(first) + X87Reg(second);
    const double third = CrtAcos(((X87Reg(c[2]) * a[2]) + (X87Reg(c[0]) * a[0])) + (X87Reg(c[1]) * a[1]));
    const X87Reg total = sum + X87Reg(third);
    if (!(total.v >= kFullCircle)) return false;

    float *point = gRaypick_7a1cc8;
    float *normal = gRaypick_7a1c80;
    for (int i = 0; i < 3; ++i) {
        point[i] = h[i];
        normal[i] = n[i];
    }
    return true;
}

}  // namespace

void RayPick_SetEnv(const RayPickEnv *env) { g_env = env ? env : &kPortEnv; }

extern "C" {

u32 __cdecl RayPick_Node(u8 *node, RayPickMatrix m) {
    Matrix_PreMul(reinterpret_cast<const float *>(node + 0x10), m.m);
    if (!(at<u32>(node, 0) & 0x8210)) {
        g_env->transformNode(node, m.m);
        const u16 *index = at<const u16 *>(node, 0x94);
        const u8 *group = at<const u8 *>(node, 0x6c);
        for (u32 g = at<u16>(node, 0x5a); g != 0; --g, group += 0x10) {
            const u8 *face = at<const u8 *>(node, 0x70) + at<u16>(group, 4) * 8u;
            for (u32 f = at<u16>(group, 6); f != 0; --f, face += 8) {
                const float *verts = global<const float *>(0x7aecd4);
                const float *v1 = verts + index[at<u16>(face, 2)] * 3u;
                const float *v2 = verts + index[at<u16>(face, 4)] * 3u;
                const float *v3 = verts + index[at<u16>(face, 6)] * 3u;
                const float *normal = at<const float *>(node, 0x64) + (at<u16>(face, 0) & 0x7fffu) * 3u;
                if (HitFace(v1, v2, v3, normal, m.m)) return 1;
            }
        }
    }
    u32 result = 0;
    if (!(at<u32>(node, 0) & 0x20)) {
        for (u8 *child = at<u8 *>(node, 0xc); child != nullptr && result == 0; child = at<u8 *>(child, 8)) result = RayPick_Node(child, m);
    }
    return result;
}

u8 *__cdecl RayPick_Column(s32 x, s32 y, u32 alongX, float atX, float atY, float atZ, float drop, const float *direction, const float *origin, float step) {
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    if (x >= global<s32>(0x7a0314)) x = global<s32>(0x7a0314) - 1;
    if (y >= global<s32>(0x7a0374)) y = global<s32>(0x7a0374) - 1;
    u8 **const rides = reinterpret_cast<u8 **>((uintptr_t)0x7a4610);

    u32 slot = 0;
    float height = (float)g_env->surfaceHeight(x, y, &slot);
    if (slot == 0) return nullptr;
    u8 *ride = rides[slot];
    u32 cars;
    u8 *train = g_env->rideTrain(slot, &cars);
    const u32 count = train != nullptr ? cars : 1;

    for (u32 i = 0; i < count; ++i) {
        if (train != nullptr) {
            u32 id = g_env->carValue(train, count - i - 1);
            ride = rides[id];
            height = (float)g_env->surfaceHeight(x, y, &id);
        } else {
            u8 *model = at<u8 *>(at<u8 *>(ride, 8), 4);
            const u32 flags = at<u32>(model, 0x30);
            if ((flags & 0x40000) && (flags & 0x20000)) g_env->updateNormals(model);
        }

        const float along = alongX != 0 ? atX : atZ;
        double place;
        if (step > kZero) {
            double whole;
            place = std::modf((X87Reg(along) * kTenth).v, &whole);
        } else {
            place = X87_SubR(X87Reg(std::fmod((double)along, 10.0)) * kTenth, kOne).v;   // (fld 10.0, the double 0x6fe670; the CRT's fmod)
        }
        const X87Reg rest = X87Reg(kOne) - X87Reg(place);
        const X87Reg low = atY - (X87Reg(place) * drop);
        const float high = (float)((rest * drop) + atY);
        const double lowest = !(low.v >= high) ? low.v : (double)high;
        if (lowest > height) continue;
        const double highest = low.v > high ? low.v : (double)high;
        u8 *model = at<u8 *>(at<u8 *>(ride, 8), 4);
        if (!(highest >= at<float>(model, 0x84))) continue;

        gRaypick_7a1cd8 = direction;
        gRaypick_7a1cb0 = origin;
        RayPickMatrix m;
        Matrix_Identity(m.m);
        if (g_env->node(at<u8 *>(model, 0x78), m) != 0) {
            const u32 *point = reinterpret_cast<const u32 *>(&gRaypick_7a1cc8);
            const u32 *normal = reinterpret_cast<const u32 *>(&gRaypick_7a1c80);
            u32 *pointOut = gRayPick_7a1cb8;
            u32 *normalOut = gRayPick_Column_7a1ca0;
            for (int k = 0; k < 3; ++k) {
                pointOut[k] = point[k];
                normalOut[k] = normal[k];
            }
            return ride;
        }
    }
    return nullptr;
}

void __cdecl RayPick_World(float *out, u32 unused, float *out2, u32 *hitFlag) {
    (void)unused;
    // The three corners of the view, turned to the screen, then by the camera.
    const float half = (float)((X87Reg(global<float>(0x74c9b8)) * gRayPick_World_74d13c) * kHalf);
    const float scale = gSinTableScale;
    const s32 first = Fistp((float)(X87Reg(scale) * half));
    const s32 second = Fistp((float)((X87Reg(half) - kQuarterTurn) * scale));
    const u32 mask = gSinTableMask;
    const float *const sines = reinterpret_cast<const float *>((uintptr_t)0x7b0968);
    const float sine = sines[mask & (u32)first], cosine = sines[mask & (u32)second];
    const float yz[2] = {(float)((X87Reg(cosine) * 0.0f) - (X87Reg(sine) * 1.0f)), (float)((X87Reg(cosine) * 1.0f) + (X87Reg(0.0f) * sine))};
    float v1[3] = {0.0f, yz[0], yz[1]}, v2[3] = {0.0f, yz[0], yz[1]}, v3[3] = {0.0f, yz[0], yz[1]};
    const double aspect = X87_Atan2(0.75, 1.0);
    TurnZ(v1, X87Reg(kTurnDown) - X87Reg(aspect));
    TurnZ(v2, X87Reg(aspect) - X87Reg(kTurnDown));
    TurnZ(v3, X87Reg(aspect) - X87Reg(kTurnUp));
    const float *const camera = reinterpret_cast<const float *>((uintptr_t)0x790990);
    Matrix_TransformVector(camera, v1);
    Matrix_TransformVector(camera, v2);
    Matrix_TransformVector(camera, v3);
    float dir[3] = {v1[0], v1[1], v1[2]};
    for (int k = 0; k < 3; ++k) {
        v2[k] = (float)(X87Reg(v2[k]) - v1[k]);
        v3[k] = (float)(X87Reg(v3[k]) - v1[k]);
    }
    const float width = global<float>(0x7a0c9c), height = global<float>(0x7a162c);
    const X87Reg u = global<float>(0x790ac8) / (X87Reg(width) + X87Reg(width));
    const float t = (float)(global<float>(0x790acc) / (X87Reg(height) + X87Reg(height)));
    dir[0] = (float)((u * v2[0]) + v1[0]);
    dir[1] = (float)((u * v2[1]) + dir[1]);
    const X87Reg z = (u * v2[2]) + dir[2];
    dir[0] = (float)((X87Reg(t) * v3[0]) + dir[0]);
    dir[1] = (float)((X87Reg(t) * v3[1]) + dir[1]);
    dir[2] = (float)((X87Reg(t) * v3[2]) + z);

    float *const cam = reinterpret_cast<float *>((uintptr_t)0x790aa0);
    const float camX = cam[0], camY = cam[1], camZ = cam[2];
    const float dx = dir[0], dy = dir[1], dz = dir[2];
    float best = 3.40282347e+38f;   // (the bits 0x7f7fffff)
    u32 hit = 0;
    float ground[3] = {0.0f, 0.0f, 0.0f};

    const u8 *const object = global<const u8 *>(0x7a0854);
    if (object != nullptr && (at<u8>(object, 0x30) & 2)) {
        const u8 *const grid = at<const u8 *>(object, 0x6c);
        const float x0 = at<float>(grid, 8), z0 = at<float>(grid, 0xc), cellX = at<float>(grid, 0x10), cellZ = at<float>(grid, 0x14);
        const u32 nx = at<u32>(grid, 0x18), ny = at<u32>(grid, 0x1c);
        const float *const heights = at<const float *>(grid, 0x28);
        const u8 *const flags = at<const u8 *>(grid, 0x2c);

        const X87Reg ta = (X87Reg(at<float>(grid, 0x24)) - camY) / dy;
        const float xA = (float)((ta * dx) + camX);
        const X87Reg zA = (ta * dz) + camZ;
        const X87Reg tb = (X87Reg(at<float>(grid, 0x20)) - camY) / dy;
        const float zB = (float)((tb * dz) + camZ);
        const s32 x1 = Clamped(Ftol(((X87Reg(xA) - x0) / cellX).v), nx);
        const s32 x2 = Clamped(Ftol((((tb * dx + camX) - x0) / cellX).v), nx);
        const s32 xMax = x1 > x2 ? x1 : x2, xMin = x1 < x2 ? x1 : x2;
        const s32 z1 = Clamped(Ftol(((zA - z0) / cellZ).v), ny);
        const s32 z2 = Clamped(Ftol(((X87Reg(zB) - z0) / cellZ).v), ny);
        const s32 zMax = z1 > z2 ? z1 : z2, zMin = z1 < z2 ? z1 : z2;

        const bool sameZ = (Below(dx) && Below(dz)) || (Above(dx) && Above(dz));
        const bool sameY = (Below(dx) && Below(dy)) || (Above(dx) && Above(dy));
        const float ndx = (float)X87Reg(-(double)dx);
        const float planZ = (float)X87Reg(-(((X87Reg(camZ) * ndx) + (X87Reg(camX) * dz)).v));
        const float planY = (float)X87Reg(-(((X87Reg(camY) * ndx) + (X87Reg(camX) * dy)).v));

        for (s32 row = zMin; row <= zMax; ++row) {
            for (s32 col = xMin; col <= xMax; ++col) {
                const float fx = (float)col;
                const X87Reg left = (X87Reg(cellX) * fx) + x0;
                const float x0f = (float)left;
                const float x1f = (float)(left + cellX);
                float za, zb;
                if (sameZ) {
                    const X87Reg near1 = (X87Reg((double)(row + 1)) * cellZ) + z0;
                    za = (float)near1;
                    zb = (float)(near1 - cellZ);
                } else {
                    const X87Reg near1 = (X87Reg((double)row) * cellZ) + z0;
                    za = (float)near1;
                    zb = (float)(near1 + cellZ);
                }
                const X87Reg e0 = ((X87Reg(x0f) * dz) + (X87Reg(ndx) * za)) + planZ;
                const float e1 = (float)(((X87Reg(x1f) * dz) + (X87Reg(ndx) * zb)) + planZ);
                if (!Crosses(e0.v, e1)) continue;

                const u32 n1 = nx + 1;
                const float h00 = heights[(u32)row * n1 + col], h01 = heights[(u32)row * n1 + col + 1];
                const float h10 = heights[((u32)row + 1) * n1 + col], h11 = heights[((u32)row + 1) * n1 + col + 1];
                float low = !(h00 >= h01) ? h00 : h01, high = !(h00 >= h01) ? h01 : h00;
                if (!(h10 >= low)) low = h10;
                if (h10 > high) high = h10;
                if (!(h11 >= low)) low = h11;
                if (h11 > high) high = h11;
                const float x1g = (float)(X87Reg(cellX) + x0f);
                const float ya = sameY ? high : low, yb = sameY ? low : high;
                const X87Reg f0 = ((X87Reg(x0f) * dy) + (X87Reg(ndx) * ya)) + planY;
                const float f1 = (float)(((X87Reg(x1g) * dy) + (X87Reg(ndx) * yb)) + planY);
                if (!Crosses(f0.v, f1)) continue;

                // (sic: the flags by rows of nx, the heights by rows of nx + 1.)
                const u8 cut = flags[(nx * (u32)row + col) * 4];
                const float fz = (float)row, fx1 = (float)(col + 1), fz1 = (float)(row + 1);
                const float minusCellZ = (float)X87Reg(-(double)cellZ), minusCellX = (float)X87Reg(-(double)cellX);
                for (int tri = 0; tri < 2; ++tri) {
                    float px, pz, py, d, e, f, g;
                    if (cut & 2) {
                        if (tri == 0) {
                            px = (float)((X87Reg(cellX) * fx) + x0), py = h10, pz = (float)((X87Reg(cellZ) * fz1) + z0);
                            d = cellX, e = (float)(X87Reg(h11) - h10), f = (float)(X87Reg(h00) - h10), g = minusCellZ;
                        } else {
                            px = (float)((X87Reg(cellX) * fx1) + x0), py = h01, pz = (float)((X87Reg(cellZ) * fz) + z0);
                            d = minusCellX, e = (float)(X87Reg(h00) - h01), f = (float)(X87Reg(h11) - h01), g = cellZ;
                        }
                    } else {
                        if (tri == 0) {
                            px = (float)((X87Reg(cellX) * fx) + x0), py = h00, pz = (float)((X87Reg(cellZ) * fz) + z0);
                            d = cellX, e = (float)(X87Reg(h01) - h00), f = (float)(X87Reg(h10) - h00), g = cellZ;
                        } else {
                            px = (float)((X87Reg(cellX) * fx1) + x0), py = h11, pz = (float)((X87Reg(cellZ) * fz1) + z0);
                            d = minusCellX, e = (float)(X87Reg(h10) - h11), f = (float)(X87Reg(h01) - h11), g = minusCellZ;
                        }
                    }
                    // (sic: the z limits by the cell's x size.)
                    const float zLow = (float)((X87Reg(cellX) * fz) + z0);
                    const float xHigh = (float)((X87Reg(cellX) * fx1) + x0);
                    const float zHigh = (float)((X87Reg(cellX) * fz1) + z0);

                    float nX = (float)(X87Reg(g) * e);
                    float nY = (float)X87Reg(-(X87Reg(g) * d).v);
                    const X87Reg nZr = X87Reg(d) * f;
                    const X87Reg length = X87_Sqrt(((X87Reg(nX) * nX) + (X87Reg(nY) * nY)) + (nZr * nZr));
                    nX = (float)(X87Reg(nX) / length);
                    nY = (float)(X87Reg(nY) / length);
                    const float nZ = (float)(nZr / length);

                    const X87Reg plane = ((X87Reg(px) * nX) + (X87Reg(py) * nY)) + (X87Reg(pz) * nZ);
                    const X87Reg top = ((X87Reg(-plane.v) + (X87Reg(camX) * nX)) + (X87Reg(camY) * nY)) + (X87Reg(camZ) * nZ);
                    const X87Reg under = ((X87Reg(nX) * dx) + (X87Reg(nY) * dy)) + (X87Reg(nZ) * dz);
                    const float along = (float)(top / under);
                    const float hx = (float)(camX - (X87Reg(along) * dx));
                    const float hy = (float)(camY - (X87Reg(along) * dy));
                    const float hz = (float)(camZ - (X87Reg(along) * dz));
                    if (((X87Reg(cellX) * fx) + x0).v > hx) continue;
                    if (!(hz >= zLow)) continue;
                    if (hx > xHigh) continue;
                    if (hz > zHigh) continue;
                    const float size = std::fabs(along);
                    if (!(size >= best)) {
                        hit = 1;
                        best = size;
                        ground[0] = hx, ground[1] = hy, ground[2] = hz;
                    }
                }
            }
        }

        // The walk along the ray. (sic: a ray not coming down, with no ground hit, goes on for ever.)
        float px = camX, py = camY, pz = camZ;
        u8 *found = nullptr;
        const u32 alongX = !(std::fabs(dz) >= std::fabs(dx)) ? 1 : 0;
        float step, side, drop;
        if (alongX) {
            step = dx > kZero ? kStep : -kStep;
            side = (float)((X87Reg(dz) / dx) * step);
            drop = (float)((X87Reg(dy) / dx) * step);
        } else {
            step = dz > kZero ? kStep : -kStep;
            side = (float)((X87Reg(dx) / dz) * step);
            drop = (float)((X87Reg(dy) / dz) * step);
        }
        const float limit = hit ? (float)(X87Reg(drop) + ground[1]) : 0.0f;
        while (py >= limit) {
            const s32 cx = Ftol((X87Reg(px) * kTenth).v), cy = Ftol((X87Reg(pz) * kTenth).v);
            found = g_env->column(cx, cy, alongX, px, py, pz, drop, dir, cam, step);
            if (found != nullptr) break;
            const float oldX = px, oldY = py, oldZ = pz;
            if (alongX) {
                px = (float)(X87Reg(step) + px);
                py = (float)(X87Reg(drop) + py);
                pz = (float)(X87Reg(side) + pz);
            } else {
                px = (float)(X87Reg(side) + px);
                py = (float)(X87Reg(drop) + py);
                pz = (float)(X87Reg(step) + pz);
            }
            bool crossed = false;
            float was = 0.0f, now = 0.0f;
            if (alongX) {
                const X87Reg w = X87Reg(pz) * kTenth;
                if (Ftol(w.v) != cy) {
                    was = (float)(X87Reg(oldZ) * kTenth);
                    now = (float)w;
                    crossed = true;
                }
            } else {
                const X87Reg w = X87Reg(px) * kTenth;
                if (Ftol(w.v) != cx) {
                    was = (float)(X87Reg(oldX) * kTenth);
                    now = (float)w;
                    crossed = true;
                }
            }
            if (crossed) {
                double whole;
                float part;
                if (!(was >= now)) {
                    const double fraction = std::modf((double)now, &whole);
                    part = (float)X87_SubR(X87Reg(fraction) / (X87Reg(now) - was), kOne);
                } else {
                    const double fraction = std::modf((double)was, &whole);
                    part = (float)(X87Reg(fraction) / (X87Reg(was) - now));
                }
                if (px >= kZero && pz >= kZero) {
                    part = (float)(X87Reg(part) * kPastCell);
                    const X87Reg other = X87Reg(kOne) - X87Reg(part);
                    const float ix = (float)((X87Reg(part) * px) + (X87Reg(oldX) * other));
                    const float iz = (float)((X87Reg(part) * pz) + (X87Reg(oldZ) * other));
                    const float iy = (float)((X87Reg(part) * py) + (X87Reg(oldY) * other));
                    found = g_env->column(Ftol((X87Reg(ix) * kTenth).v), Ftol((X87Reg(iz) * kTenth).v), alongX, ix, iy, iz, drop, dir, cam, step);
                    if (found != nullptr) break;
                }
            }
        }
        global<u8 *>(0x7a1cd4) = found;
        g_env->setTarget(found);
    }

    if (hitFlag != nullptr) *hitFlag = hit;
    if (out == nullptr) return;
    bool useTarget = false;
    const float *const point = reinterpret_cast<const float *>(&gRayPick_7a1cb8);
    if (global<u8 *>(0x7a1cd4) != nullptr) {
        if (!hit) {
            useTarget = true;
        } else {
            const float groundDistance = (float)Vec3_Distance(ground, cam);
            useTarget = !(Vec3_Distance(point, cam) >= groundDistance);
        }
    }
    const float *const from = useTarget ? point : hit ? ground : nullptr;
    if (from != nullptr) {
        for (int k = 0; k < 3; ++k) out[k] = from[k];
        if (out2 != nullptr) {
            for (int k = 0; k < 3; ++k) out2[k] = from[k];
        }
    }
}
}
